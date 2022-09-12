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
    m_status(true)
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
    auto element = document.nodes.Elements()[m_nodeIterator];
    auto mesh = document.meshes[element.meshId];
    
    std::vector<GVertex> vertices;
    std::unordered_set<GVertex, HashGVertex, EqualsGVertex> uniqueVertices;
    std::vector<u32> indices;

    for (auto primitive : mesh.primitives)
    {
        std::string accessorId;

        std::vector<XMFLOAT3> positions;
        std::vector<XMFLOAT3> normals;
        std::vector<XMFLOAT2> textureCoords;
        std::vector<XMINT4> bones;
        std::vector<XMFLOAT4> weights;

        if (primitive.TryGetAttributeAccessorId(glTF::ACCESSOR_POSITION, accessorId))
        {
            const auto& accessor = document.accessors.Get(accessorId);
            auto data = reader->ReadBinaryData<f32>(document, accessor);

            for (u32 i = 0; i < data.size(); i += 3)
            {
                positions.push_back(XMFLOAT3(
                    data[i],
                    data[i + 1],
                    data[i + 2]
                ));
            }
        }

        if (primitive.TryGetAttributeAccessorId(glTF::ACCESSOR_NORMAL, accessorId))
        {
            const auto& accessor = document.accessors.Get(accessorId);
            auto data = reader->ReadBinaryData<f32>(document, accessor);

            for (u32 i = 0; i < data.size(); i += 3)
            {
                normals.push_back(XMFLOAT3(
                    data[i],
                    data[i + 1],
                    data[i + 2]
                ));
            }
        }

        if (primitive.TryGetAttributeAccessorId(glTF::ACCESSOR_TEXCOORD_0, accessorId))
        {
            const auto& accessor = document.accessors.Get(accessorId);
            auto data = reader->ReadBinaryData<f32>(document, accessor);

            for (u32 i = 0; i < data.size(); i += 2)
            {
                textureCoords.push_back(XMFLOAT2(
                    data[i],
                    data[i + 1]
                ));
            }
        }

        if (primitive.TryGetAttributeAccessorId(glTF::ACCESSOR_JOINTS_0, accessorId))
        {
            const auto& accessor = document.accessors.Get(accessorId);
            auto data = reader->ReadBinaryData<u8>(document, accessor);

            for (u32 i = 0; i < data.size(); i += 4)
            {
                bones.push_back(XMINT4(
                    data[i],
                    data[i + 2],
                    data[i + 3],
                    data[i + 4]
                ));
            }
        }

        if (primitive.TryGetAttributeAccessorId(glTF::ACCESSOR_WEIGHTS_0, accessorId))
        {
            const auto& accessor = document.accessors.Get(accessorId);
            auto data = reader->ReadBinaryData<f32>(document, accessor);

            for (u32 i = 0; i < data.size(); i += 4)
            {
                weights.push_back(XMFLOAT4(
                    data[i],
                    data[i + 2],
                    data[i + 3],
                    data[i + 4]
                ));
            }
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
            auto indexedVertex = std::find(uniqueVertices.begin(), uniqueVertices.end(), vertex);
            if (uniqueVertices.size() == 0 || indexedVertex == uniqueVertices.end())
            {
                //Insert Brand new vertex, never before seen
                uniqueVertices.insert(vertex);
                vertices.push_back(vertex);
                indices.push_back(uniqueVertices.size() - 1);
            }
            else
            {
                auto index = std::distance(uniqueVertices.begin(), indexedVertex);
                indices.push_back((u32)index);
            }
        }
    }
}


void GltfMeshFactory::createMeshes(std::vector<MeshActor>& meshActors)
{

}