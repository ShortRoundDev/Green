#include "Mesh.h"

#include "GraphicsManager.h"
#include "GameManager.h"
#include "Logger.h"
#include "Texture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "reactphysics3d/reactphysics3d.h"

#include <unordered_set>

using namespace reactphysics3d;

static ::Logger logger = CreateLogger("Mesh");

struct HashXMFloat3
{
public:
    size_t operator()(const XMFLOAT3& vector) const
    {
               // Evil
        return (std::bit_cast<u32>(vector.x) * 73856093) ^
               (std::bit_cast<u32>(vector.y) * 19349663) ^
               (std::bit_cast<u32>(vector.z) * 83492791);
    }
};

struct EqualsXMFloat3
{
public:
    bool operator()(const XMFLOAT3& a, const XMFLOAT3& b) const
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};

bool Mesh::createFromFile(
    std::string path,
    Mesh** meshes, size_t* totalMeshes,
    ConvexMeshShape*** physicsMeshes, size_t* totalPhysicsMeshes,
    GameManager* gameManager
)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (scene == NULL || gameManager == NULL)
    {
        *physicsMeshes = NULL;
        *totalPhysicsMeshes = 0;
        *meshes = NULL;
        *totalMeshes = 0;
        return false;
    }

    std::vector<Mesh> outMeshes = std::vector<Mesh>();

    auto root = scene->mRootNode;
    *totalPhysicsMeshes = root->mNumChildren;
    *physicsMeshes = new ConvexMeshShape*[*totalPhysicsMeshes];
    for (u32 i = 0; i < root->mNumChildren; i++)
    {
        auto child = root->mChildren[i];
        logger.info("%d: Meshes: %d", i, child->mNumMeshes);
    }

    // For each NODE in SCENE
    for (u32 i = 0; i < root->mNumChildren; i++)
    {
        auto xmFloat3Comparator = [](XMFLOAT3 a, XMFLOAT3 b) { return a.x == b.x && a.y == b.y && a.z == b.z; };

        std::vector<XMFLOAT3> nodeVertices;
        std::unordered_set<XMFLOAT3, HashXMFloat3, EqualsXMFloat3> uniqueNodeVertices;
        std::vector<u32> nodeIndices = std::vector<u32>();

        std::vector<PolygonVertexArray::PolygonFace> nodeFaces = std::vector<PolygonVertexArray::PolygonFace>();

        auto child = root->mChildren[i];
        u32 faceIndexOffset = 0;

        //For each MESH in NODE
        for (u32 j = 0; j < child->mNumMeshes; j++)
        {
            std::vector<GVertex> meshVertices = std::vector<GVertex>();
            std::vector<u32> meshIndices = std::vector<u32>();

            auto mesh = scene->mMeshes[child->mMeshes[j]];

            //For each VERTEX in MESH
            for (u32 k = 0; k < mesh->mNumVertices; k++)
            {
                f32 u = 0.0f, v = 0.0f;
                if (mesh->mTextureCoords[0])
                {
                    u = mesh->mTextureCoords[0][k].x;
                    v = mesh->mTextureCoords[0][k].y;
                }

                GVertex vertex = CreateVertex(
                    mesh->mVertices[k].x,
                    mesh->mVertices[k].y,
                    mesh->mVertices[k].z,
                    mesh->mNormals[k].x,
                    mesh->mNormals[k].y,
                    mesh->mNormals[k].z,
                    -u,
                    v
                );

                auto nodeVertex = XMFLOAT3(mesh->mVertices[k].x, mesh->mVertices[k].y, mesh->mVertices[k].z);
                if (uniqueNodeVertices.find(nodeVertex) == uniqueNodeVertices.end()) // not found
                {
                    uniqueNodeVertices.insert(nodeVertex);
                    nodeVertices.push_back(nodeVertex);
                }

                meshVertices.push_back(vertex);
            }
            
            //For each FACE in MESH
            for (u32 k = 0; k < mesh->mNumFaces; k++)
            {
                auto face = mesh->mFaces[k];
                
                //For each INDEX in FACE
                PolygonVertexArray::PolygonFace nodeFace = PolygonVertexArray::PolygonFace();
                nodeFace.indexBase = faceIndexOffset;
                nodeFace.nbVertices = face.mNumIndices;
                nodeFaces.push_back(nodeFace);
                for (u32 l = 0; l < face.mNumIndices; l++)
                {
                    meshIndices.push_back(face.mIndices[l]);

                    //remap node index
                    auto uniquePos = meshVertices[face.mIndices[l]];
                    auto vertexIterator = std::find_if(nodeVertices.begin(), nodeVertices.end(), [uniquePos](const XMFLOAT3& a) {
                        return a.x == uniquePos.pos.x && a.y == uniquePos.pos.y && a.z == uniquePos.pos.z;
                    });
                    auto index = std::distance(nodeVertices.begin(), vertexIterator);
                    
                    nodeIndices.push_back(index);
                }
                faceIndexOffset = nodeIndices.size();                
            }

            //Get texture from material list on mesh
            Texture* texture = NULL;
            int texIdx = 0;
            if (mesh->mMaterialIndex >= 0)
            {
                aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
                auto diffuseCount = material->GetTextureCount(aiTextureType_DIFFUSE);
                if (diffuseCount > 0)
                {
                    aiString str;
                    material->GetTexture(aiTextureType_DIFFUSE, 0, &str);

                    texture = Graphics.getTexture(std::string(str.C_Str()));
                }
            }

            // Create MESH for Rendering
            Mesh _mesh = Mesh();
            _mesh.initialize(
                meshVertices,
                meshVertices.size(),
                meshIndices,
                meshIndices.size(),
                texture
            );
            outMeshes.push_back(_mesh);
        }

        //Create NODE for Physics
        PolygonVertexArray* nodeVertexArray = new PolygonVertexArray(
            nodeVertices.size(),
            nodeVertices.data(),
            sizeof(XMFLOAT3),
            nodeIndices.data(),
            sizeof(u32),
            nodeFaces.size(),
            nodeFaces.data(),
            PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
            PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE
        );

        PolyhedronMesh* nodeMesh = gameManager->getPhysicsCommon()->createPolyhedronMesh(nodeVertexArray);
        (*physicsMeshes)[i] = gameManager->getPhysicsCommon()->createConvexMeshShape(nodeMesh);
        
        *totalMeshes = outMeshes.size();
        *meshes = outMeshes.data();
    }

    /*for (u32 i = 0; i < scene->mNumMeshes; i++)
    {
        auto mesh = scene->mMeshes[i];
        
        size_t vertCount = mesh->mNumVertices;
        std::vector<GVertex> vertices = std::vector<GVertex>(vertCount);
        std::vector<u32> indices = std::vector<u32>();
       
        for (u32 j = 0;  j< vertCount; j++)
        {
            f32 u = 0.0f, v = 0.0f;
            if (mesh->mTextureCoords[0])
            {
                u = mesh->mTextureCoords[0][j].x;
                v = mesh->mTextureCoords[0][j].y;
            }
            vertices[j] = CreateVertex(
                mesh->mVertices[j].x,
                mesh->mVertices[j].y,
                mesh->mVertices[j].z,
                mesh->mNormals[j].x,
                mesh->mNormals[j].y,
                mesh->mNormals[j].z,
                -u,
                v
            );
        }

        size_t indexCount = 0;
        for (u32 j = 0; j < mesh->mNumFaces; j++)
        {
            auto face = mesh->mFaces[j];
            indexCount += face.mNumIndices;
            for (u32 k = 0; k < face.mNumIndices; k++)
            {
                indices.push_back(face.mIndices[k]);
            }
        }

        Texture* texture = NULL;
        int texIdx = 0;
        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            auto diffuseCount = material->GetTextureCount(aiTextureType_DIFFUSE);
            if (diffuseCount > 0)
            {
                aiString str;
                material->GetTexture(aiTextureType_DIFFUSE, 0, &str);

                texture = Graphics.getTexture(std::string(str.C_Str()));
            }
        }

        (*meshes)[i].initialize(
            vertices,
            vertCount,
            indices,
            indexCount,
            texture
        );
    }*/
    return true;
}

void Mesh::draw()
{
    m_texture->use();
    UINT stride = sizeof(GVertex);
    UINT offset = 0;

    auto context = Graphics.getContext();

    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(m_indexCount, 0, 0);
}

Mesh::Mesh()
{

}

Mesh::~Mesh()
{

}

bool Mesh::initialize(
    const std::vector<GVertex>& vertices,
    size_t vertCount,
    const std::vector<u32>& indices,
    size_t indexCount,
    Texture* texture
)
{
    m_texture = texture;

    const GVertex* pVertices = vertices.data();
    const u32* pIndices = indices.data();

    m_vertCount = (u32)vertCount;
    m_indexCount = (u32)indexCount;

    D3D11_BUFFER_DESC vertexDesc;
    vertexDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexDesc.ByteWidth = (UINT)(vertCount * sizeof(GVertex));
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.CPUAccessFlags = 0;
    vertexDesc.MiscFlags = 0;
    vertexDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexData;
    vertexData.pSysMem = pVertices;
    vertexData.SysMemPitch = 0;
    vertexData.SysMemSlicePitch = 0;

    HRESULT result = Graphics.getDevice()->CreateBuffer(&vertexDesc, &vertexData, m_vertexBuffer.GetAddressOf());
    if (FAILED(result))
    {
        m_status = false;
        logger.err("Failed to create vertex buffer!");
        return false;
    }

    D3D11_BUFFER_DESC indexDesc;
    indexDesc.Usage = D3D11_USAGE_DEFAULT;
    indexDesc.ByteWidth = (u32)(sizeof(u64) * indexCount);
    indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexDesc.CPUAccessFlags = 0;
    indexDesc.MiscFlags = 0;
    indexDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA indexData;
    indexData.pSysMem = pIndices;
    indexData.SysMemPitch = 0;
    indexData.SysMemSlicePitch = 0;

    result = Graphics.getDevice()->CreateBuffer(&indexDesc, &indexData, m_indexBuffer.GetAddressOf());
    if (FAILED(result))
    {
        m_status = false;
        logger.err("Failed to create index buffer!");
        return false;
    }

    m_status = true;
    return true;
}

void Mesh::concatenateVertices(
    std::vector<GVertex>& out,
    const std::vector<GVertex>& a,
    const std::vector<GVertex>& b
)
{
    for (u32 i = 0; i < a.size(); i++)
    {
        out.push_back(a[i]);
    }
    for (u32 i = 0; i < b.size(); i++)
    {
        out.push_back(b[i]);
    }
}

void Mesh::concatenateIndices(
    std::vector<u32>& out,
    const std::vector<u32>& a,
    const std::vector<u32>& b
)
{
    size_t initialSize = out.size();
    for (u32 i = 0; i < a.size(); i++)
    {
        out.push_back(a[i] + initialSize);
    }
    initialSize = out.size();
    for (u32 i = 0; i < b.size(); i++)
    {
        out.push_back(b[i] + initialSize);
    }
}