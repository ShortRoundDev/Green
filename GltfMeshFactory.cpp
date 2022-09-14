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

#include <crossguid/guid.hpp>

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
    
    std::vector<GVertex> vertices;
    std::vector<u32> indices;

    for (auto primitive : mesh.primitives)
    {
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
            return false;
        }

        auto image = document.images[texture.imageId];
            
        auto data = reader->ReadBinaryData(document, image);
        std::string name = image.name;
        if (name.length() == 0)
        {
            name = xg::newGuid().str();
        }

        Texture *greenTexture = Graphics.lazyLoadTexture(image.name, data.data(), data.size());

        meshActor.mesh = new Mesh(
            vertices,
            vertices.size(),
            indices,
            indices.size(),
            greenTexture
        );

        //auto skin = document.skins[element.skinId];
        

        return true;
    }
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


void GltfMeshFactory::createMeshes(std::vector<MeshActor>& meshActors)
{

}