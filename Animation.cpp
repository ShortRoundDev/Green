#include "Animation.h"

#include "Mesh.h"
#include "Util.h"

#include "Logger.h"

#include <GLTFSDK/Deserialize.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/IStreamReader.h>

#include <assimp/anim.h>

#include <fstream>
#include <sstream>

using namespace Microsoft;

static Logger logger = CreateLogger("Animation");

/*class StreamReader : public glTF::IStreamReader
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

};*/

::Animation::Animation()
{

}

::Animation::~Animation()
{

}

::Animation::Animation(std::string animationPath, ::Mesh* mesh)
{
    /*Assimp::Importer importer;
    auto scene = importer.ReadFile(animationPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
    if (scene->mNumAnimations == 0)
    {

        return;
    }
    auto animation = scene->mAnimations[0];
    m_duration = (f32)animation->mDuration;
    m_ticksPerSecond = (i32)animation->mTicksPerSecond;

    readHierarchyData(m_rootNode, scene->mRootNode);
    readMissingBones(animation, mesh);
    */

    /*auto sreader = std::make_unique<StreamReader>();
    auto stream = sreader->GetInputStream(animationPath);
    glTF::GLTFResourceReader reader(std::move(sreader));

    std::string manifest;

    std::stringstream manifestStream;
    manifestStream << stream->rdbuf();
    manifest = manifestStream.str();

    std::vector<GVertex> vertices;

    // ========================== LEFT OFF HERE VVV ============================ //
    auto doc = glTF::Deserialize(manifest);

    std::vector<XMMATRIX> matrices;

    auto bindMatrixAccessor = doc.accessors.Get(doc.skins[0].inverseBindMatricesAccessorId);
    auto matrixData = reader.ReadBinaryData<float>(doc, bindMatrixAccessor);
    for (int i = 0; i < matrixData.size(); i += 16)
    {
        matrices.push_back(XMMatrixSet(
            matrixData[i + 0],
            matrixData[i + 1],
            matrixData[i + 2],
            matrixData[i + 3],
            matrixData[i + 4],
            matrixData[i + 5],
            matrixData[i + 6],
            matrixData[i + 7],
            matrixData[i + 8],
            matrixData[i + 9],
            matrixData[i + 10],
            matrixData[i + 11],
            matrixData[i + 12],
            matrixData[i + 13],
            matrixData[i + 14],
            matrixData[i + 15]
        ));
    }

    for (auto mesh : doc.meshes.Elements())
    {
        logger.info("Mesh ID: %d", mesh.id);
        for (auto primitive : mesh.primitives)
        {
            std::string accessor;
            if (primitive.TryGetAttributeAccessorId(glTF::ACCESSOR_POSITION, accessor))
            {
                auto access = doc.accessors.Get(accessor);

                auto data = reader.ReadBinaryData<f32>(doc, access);

                for (int i = 0; i < data.size(); i += 3)
                {
                    XMFLOAT3 pos;

                    pos.x = data[i];
                    pos.y = data[i + 1];
                    pos.z = data[i + 2];
                    
                    GVertex vertex;
                    vertex.pos = pos;

                    vertices.push_back(vertex);
                }
            }

            if (primitive.TryGetAttributeAccessorId(glTF::ACCESSOR_NORMAL, accessor))
            {
                auto access = doc.accessors.Get(accessor);

                auto data = reader.ReadBinaryData<f32>(doc, access);
                
                for (int i = 0; i < data.size(); i += 3)
                {
                    XMFLOAT3 normal;

                    normal.x = data[i];
                    normal.y = data[i + 1];
                    normal.z = data[i + 2];

                    vertices[i/3].normal = normal;
                }
            }

            if (primitive.TryGetAttributeAccessorId(glTF::ACCESSOR_TEXCOORD_0, accessor))
            {
                auto access = doc.accessors.Get(accessor);

                auto data = reader.ReadBinaryData<f32>(doc, access);

                for (int i = 0; i < data.size(); i += 2)
                {
                    XMFLOAT2 tex;

                    tex.x = data[i];
                    tex.y = data[i + 1];                    

                    vertices[i/2].tex = tex;
                }
            }

            if (primitive.TryGetAttributeAccessorId(glTF::ACCESSOR_JOINTS_0, accessor))
            {
                auto access = doc.accessors.Get(accessor);

                auto data = reader.ReadBinaryData<u8>(doc, access);

                for (int i = 0; i < data.size(); i += 4)
                {
                    XMINT4 bones;

                    bones.x = data[i];
                    bones.y = data[i + 1];
                    bones.z = data[i + 2];
                    bones.w = data[i + 3];

                    vertices[i/4].bones = bones;
                }
            }

            if (primitive.TryGetAttributeAccessorId(glTF::ACCESSOR_WEIGHTS_0, accessor))
            {
                auto access = doc.accessors.Get(accessor);

                auto data = reader.ReadBinaryData<f32>(doc, access);

                for (int i = 0; i < data.size(); i += 4)
                {
                    XMFLOAT4 weights;

                    weights.x = data[i];
                    weights.y = data[i + 1];
                    weights.z = data[i + 2];
                    weights.w = data[i + 3];

                    vertices[i/4].weights = weights;
                }
            }
        }
    }*/
};

Bone* Animation::findBone(const std::string& name)
{
    auto res = m_boneInfoMap.find(name);
    if (res == m_boneInfoMap.end())
    {
        return nullptr;
    }
    return m_bones.data() + res->second.id; // woop woop
}

i32 Animation::getTicksPerSecond()
{
    return m_ticksPerSecond;
}

f32 Animation::getDuration()
{
    return m_duration;
}

const AnimNodeData& ::Animation::getRootNode()
{
    return m_rootNode;
}

const std::map<std::string, BoneInfo>& ::Animation::getBoneInfoMap()
{
    return m_boneInfoMap;
}

void Animation::readHierarchyData(AnimNodeData& node, const aiNode* src)
{
    node.name = src->mName.data;
    convertAiMatrixToXMMatrix(src->mTransformation, node.transform);
    node.childCount = src->mNumChildren;

    for (u32 i = 0; i < src->mNumChildren; i++)
    {
        AnimNodeData newData;
        readHierarchyData(newData, src->mChildren[i]);
        node.children.push_back(newData);
    }
}

void ::Animation::readMissingBones(const aiAnimation* animation, ::Mesh* mesh)
{
    u32 size = animation->mNumChannels;
    auto& boneInfoMap = mesh->getBoneInfoMap();

    i32 boneCount = mesh->getBoneCounter();

    for (u32 i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
            mesh->setBoneCounter(boneCount);
        }
        m_bones.push_back(Bone(boneName, boneInfoMap[boneName].id, channel));
    }

    m_boneInfoMap = boneInfoMap;
}