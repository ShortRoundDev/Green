#include "Mesh.h"

#include "GraphicsManager.h"
#include "Logger.h"
#include "Texture.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

static Logger logger = CreateLogger("Mesh");

bool Mesh::createFromFile(std::string path, Mesh** meshes, size_t* totalMeshes)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (scene == NULL)
    {
        *meshes = NULL;
        *totalMeshes = 0;
        return false;
    }
    *totalMeshes = scene->mNumMeshes;
    *meshes = new Mesh[*totalMeshes];

    for (u32 i = 0; i < scene->mNumMeshes; i++)
    {
        auto node = scene->mRootNode;
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
    }
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
