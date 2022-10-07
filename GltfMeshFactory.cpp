#include "GltfMeshFactory.h"

#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/IStreamReader.h>
#include <GLTFSDK/Deserialize.h>

#include <fstream>
#include <filesystem>
#include <sstream>

#include "Logger.h"
#include "GVertex.h"
#include "GraphicsManager.h"
#include "Mesh.h"

#include "AnimationChannel.h"

#include <crossguid/guid.hpp>
#include <tuple>
#include "AnimationJoint.h"

static Logger logger = CreateLogger("GltfMeshFactory");

#pragma region STREAMREADER
static class StreamReader : public glTF::IStreamReader
{
public:
    StreamReader()
    {
    }

    std::shared_ptr<std::istream> GetInputStream(const std::string& filename) const override
    {
        auto stream = std::make_shared<std::ifstream>(filename);

        // Check if the stream has no errors and is ready for I/O operations
        if (!stream || !(*stream))
        {
            throw std::runtime_error("Unable to create a valid input stream for uri: " + filename);
        }

        return stream;
    }

};
#pragma endregion

GltfMeshFactory::GltfMeshFactory(std::string path, GameManager* game) :
    m_game(game),
    m_nodeIterator(0),
    m_status(true),
    m_path(path)
{

}

bool GltfMeshFactory::createMesh(MeshActor& meshActor)
{
    auto streamReader = std::make_unique<StreamReader>();

    auto stream = streamReader->GetInputStream(m_path);
    auto resourceReader = std::make_unique<glTF::GLTFResourceReader>(std::move(streamReader));

    std::stringstream manifestStream;
    manifestStream << stream->rdbuf();

    std::string manifest = manifestStream.str();

    glTF::Document document;

    try
    {
        document = glTF::Deserialize(manifest);
    }
    catch (const glTF::GLTFException& e)
    {
        logger.err("glTF::Deserialize failed! Got %s", e.what());
        return false;
    }
    return createMesh(meshActor, document, resourceReader.get());
}

bool GltfMeshFactory::createMesh(MeshActor& meshActor, const glTF::Document& document, glTF::GLTFResourceReader* reader)
{
    if (m_nodeIterator >= document.nodes.Size())
    {
        m_status = false;
        return false;
    }
    glTF::Node element;
    do
    {
        //nodes often have no meshes. Those are cameras, lights, etc. Iterate through nodes until
        //we reach one with a mesh.
        element = document.nodes.Elements()[m_nodeIterator];
        m_nodeIterator++;
    } while (element.meshId == "");

    auto mesh = document.meshes[element.meshId];
    
    if (mesh.primitives.size() == 0)
    {   
        m_nodeIterator++;
        return false;
    }

    auto primitive = mesh.primitives[0];
    meshFromPrimitive(meshActor.mesh, primitive, document, reader);
    animationFromSkin(meshActor.animations, document.skins[element.skinId], document, reader);

    return true;
}

template<typename T, typename U, auto V>
void GltfMeshFactory::getAttributeData(
    const glTF::Document& document,
    const glTF::MeshPrimitive& primitive,
    glTF::GLTFResourceReader* reader,
    std::string accessor,
    std::vector<U>& list
)
{
    getAttributeData<T, T, U, V>(document, primitive, reader, accessor, list);
}

template<typename T, typename W, typename U, auto V>
void GltfMeshFactory::getAttributeData(
    const glTF::Document& document,
    const glTF::MeshPrimitive& primitive,
    glTF::GLTFResourceReader* reader,
    std::string accessor,
    std::vector<U>& list
)
{
    static thread_local W buffer[V];
    std::string accessorId;
    if (primitive.TryGetAttributeAccessorId(accessor, accessorId))
    {
        const auto& accessor = document.accessors.Get(accessorId);
        auto data = reader->ReadBinaryData<T>(document, accessor);

        for (u32 i = 0; i < data.size(); i += V)
        {
            for (u32 j = 0; j < V; j++)
            {
                buffer[j] = (W)data[i + j];
            }

            list.push_back(U(buffer));
        }
    }
}

void GltfMeshFactory::meshFromPrimitive(
    Mesh*& mesh,
    const glTF::MeshPrimitive& primitive,
    const glTF::Document& document,
    glTF::GLTFResourceReader* reader
)
{
    std::vector<GVertex> vertices;
    std::vector<u32> indices;

    std::string accessorId;

    std::vector<XMFLOAT3> positions;
    std::vector<XMFLOAT3> normals;
    std::vector<XMFLOAT2> textureCoords;
    std::vector<XMINT4> bones;
    std::vector<XMFLOAT4> weights;

    getAttributeData<f32, XMFLOAT3, 3>(document, primitive, reader, glTF::ACCESSOR_POSITION, positions);
    getAttributeData<f32, XMFLOAT3, 3>(document, primitive, reader, glTF::ACCESSOR_NORMAL, normals);
    getAttributeData<f32, XMFLOAT2, 2>(document, primitive, reader, glTF::ACCESSOR_TEXCOORD_0, textureCoords);
    getAttributeData<f32, XMFLOAT4, 4>(document, primitive, reader, glTF::ACCESSOR_WEIGHTS_0, weights);
    getAttributeData<u8, i32, XMINT4, 4>(document, primitive, reader, glTF::ACCESSOR_JOINTS_0, bones);

    const auto& accessor = document.accessors.Get(primitive.indicesAccessorId);
    auto indexData = reader->ReadBinaryData<u16>(document, accessor);

    for (u32 i = 0; i < indexData.size(); i++)
    {
        indices.push_back((u32)indexData[i]);
    }

    for (u32 i = 0; i < positions.size(); i++)
    {
        GVertex vertex(
            positions[i],
            normals[i],
            textureCoords[i],
            bones[i],
            weights[i]
        );

        vertices.push_back(GVertex(
            positions[i],
            normals[i],
            textureCoords[i],
            bones[i],
            weights[i]
        ));
    }

    auto material = document.materials[primitive.materialId];

    auto textures = material.GetTextures();
    glTF::Texture texture;
    bool found = false;
    for (i32 i = 0; i < textures.size(); i++)
    {
        if (textures[i].second == glTF::TextureType::BaseColor)
        {
            found = true;
            texture = document.textures[textures[i].first];
        }
    }
    if (!found)
    {
        mesh = nullptr;
        return;
    }

    auto image = document.images[texture.imageId];

    auto data = reader->ReadBinaryData(document, image);
    std::string name = image.name;
    if (name.length() == 0)
    {
        name = xg::newGuid().str();
    }

    Texture* greenTexture = Graphics.lazyLoadTexture(image.name, data.data(), data.size());

    mesh = new Mesh(
        vertices,
        vertices.size(),
        indices,
        indices.size(),
        greenTexture
    );
}

void GltfMeshFactory::createMeshes(std::vector<MeshActor>& meshActors)
{

}

void GltfMeshFactory::animationFromSkin(
    std::map<std::string, AnimationSkeleton*>& animations,
    const glTF::Skin& skin,
    const glTF::Document& document,
    glTF::GLTFResourceReader* reader
)
{
    sz jointSize = skin.jointIds.size();

    logger.info("%d", document.animations.Size());

    for (u32 i = 0; i < document.animations.Size(); i++)
    {
        std::vector<std::tuple<
            std::vector<AnimationKeyFrame>,
            std::vector<AnimationKeyFrame>,
            std::vector<AnimationKeyFrame>
        >> timelineBuffer(jointSize);
        
        glTF::Animation animation = document.animations[i];
        for (u32 j = 0; j < animation.channels.Size(); j++)
        {
            glTF::AnimationChannel channel = animation.channels[j];
            glTF::AnimationSampler sampler = animation.samplers[channel.samplerId];
            i32 jointIndex =  (i32)std::distance(
                skin.jointIds.begin(),
                std::find(
                    skin.jointIds.begin(), skin.jointIds.end(),
                    channel.target.nodeId
                )
            );
            glTF::TargetPath path = channel.target.path;
            
            auto& jointChannelBuffer = timelineBuffer[jointIndex]; // tuple

            std::vector<AnimationKeyFrame>* channelBuffer = nullptr;
            switch (path)
            {
            case glTF::TARGET_TRANSLATION:
            {
                channelBuffer = &std::get<0>(jointChannelBuffer);
                break;
            }
            case glTF::TARGET_SCALE:
            {
                channelBuffer = &std::get<1>(jointChannelBuffer);
                break;
            }
            case glTF::TARGET_ROTATION:
            {
                channelBuffer = &std::get<2>(jointChannelBuffer);
                break;
            }
            }

            //glTF::AnimationSampler sampler = animation.samplers[j];
            auto inputId = sampler.inputAccessorId;
            auto outputId = sampler.outputAccessorId;

            auto input = document.accessors[inputId];
            auto output = document.accessors[outputId];

            auto time = reader->ReadBinaryData<f32>(document, input);
            auto values = reader->ReadBinaryData<f32>(document, output);

            u32 vectorDisposition = values.size() / time.size(); // 3 or 4
            for (u32 k = 0; k < time.size(); k++)
            {
                f32 vecBuffer[4] = { 0, 0, 0, 1.0f };
                for (u32 l = 0; l < vectorDisposition; l++)
                {
                    vecBuffer[l] = values[k * vectorDisposition + l];
                }

                channelBuffer->push_back(AnimationKeyFrame(
                    time[k],
                    XMVectorSet(vecBuffer[0], vecBuffer[1], vecBuffer[2], vecBuffer[3])
                ));
            }
        }

        std::vector<AnimationChannel> timeline;
        for (u32 j = 0; j < timelineBuffer.size(); j++)
        {
            timeline.push_back(AnimationChannel(
                std::get<0>(timelineBuffer[j]),
                std::get<1>(timelineBuffer[j]),
                std::get<2>(timelineBuffer[j])
            ));
        }

        u8* tree = new u8[jointSize];

        tree[0] = 0;
        for (u32 j = 0; j < jointSize; j++)
        {
            auto jointNode = document.nodes[skin.jointIds[j]];
            for (u32 k = 0; k < jointNode.children.size(); k++)
            {
                auto child = jointNode.children[k];
                auto childSkinJointItr = std::find(skin.jointIds.begin(), skin.jointIds.end(), child);
                i64 childSkinJointIndex = (i64)std::distance(skin.jointIds.begin(), childSkinJointItr);

                tree[childSkinJointIndex] = j;
            }
        }

        std::vector<XMMATRIX> inverseBindMatrices;
        auto invBindAccessor = document.accessors[skin.inverseBindMatricesAccessorId];
        auto invBindData = reader->ReadBinaryData<f32>(document, invBindAccessor);
        for (u32 j = 0; j < invBindData.size(); j += 16)
        {
            inverseBindMatrices.push_back(XMMatrixSet(
                invBindData[j +  0], invBindData[j +  1], invBindData[j +  2], invBindData[j +  3],
                invBindData[j +  4], invBindData[j +  5], invBindData[j +  6], invBindData[j +  7],
                invBindData[j +  8], invBindData[j +  9], invBindData[j + 10], invBindData[j + 11],
                invBindData[j + 12], invBindData[j + 13], invBindData[j + 14], invBindData[j + 15]
            ));
        }

        std::vector<AnimationJoint> finalJoints;
        for (u32 j = 0; j < timeline.size(); j++)
        {
            auto node = document.nodes[skin.jointIds[j]];
            
            
            auto glTranslate = node.translation;
            auto glRotate = node.rotation;
            auto glScale = node.scale;
            XMVECTOR translate(XMVectorSet(glTranslate.x, glTranslate.y, glTranslate.z, 1.0f));
            XMVECTOR rotate(XMVectorSet(glRotate.x, glRotate.y, glRotate.z, glRotate.w));
            XMVECTOR scale(XMVectorSet(glScale.x, glScale.y, glScale.z, 1.0f));
            
            XMMATRIX localMatrix = XMMatrixAffineTransformation(scale, g_XMZero, rotate, translate);

            finalJoints.push_back(AnimationJoint(
                j, inverseBindMatrices[j], timeline[j], localMatrix
            ));
        }

        animations[animation.name] = new AnimationSkeleton(tree, std::move(finalJoints));
    }
}