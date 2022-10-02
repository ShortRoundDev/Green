#include "Mesh.h"

#include "GraphicsManager.h"
#include "GameManager.h"
#include "Logger.h"
#include "Texture.h"
#include "ILight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "MeshViewModel.h"
#include "NavMesh.h"
#include "Util.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "nlohmann/json.hpp"

#include <unordered_set>
#include <fstream>

static ::Logger logger = CreateLogger("Mesh");

using json = nlohmann::json;

struct HashPxVector3
{
public:
    sz operator()(const PxVec3& vector) const
    {
        return (std::bit_cast<u64>((f64)vector.x) * 73856093ul) ^
               (std::bit_cast<u64>((f64)vector.y) * 19349663ul) ^
               (std::bit_cast<u64>((f64)vector.z) * 83492791ul);
    }
};

struct EqualsPxVector3
{
public:
    bool operator()(const PxVec3& a, const PxVec3& b) const
    {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};

bool Mesh::createBbox(
    AABB aabb,
    Texture* texture,
    Mesh*& mesh
)
{
    auto& min = aabb.getMin();
    auto& max = aabb.getMax();

    f32
        x1 = min.x,
        y1 = min.y,
        z1 = min.z;

    f32
        x2 = max.x,// -((max.x - min.x) / 2.0f),
        y2 = max.y,// -((max.y - min.y) / 2.0f),
        z2 = max.z;// -((max.z - min.z) / 2.0f);

    std::vector<GVertex> vertices = std::vector<GVertex>({
        GVertex(x1, y1, z1, 0, 0, 0, 0, 0), // min
        GVertex(x1, y2, z1, 0, 0, 0, 0, 0),
        GVertex(x2, y2, z1, 0, 0, 0, 0, 0),
        GVertex(x2, y1, z1, 0, 0, 0, 0, 0),
        GVertex(x2, y2, z2, 0, 0, 0, 0, 0), // max
        GVertex(x2, y1, z2, 0, 0, 0, 0, 0),
        GVertex(x1, y1, z2, 0, 0, 0, 0, 0),
        GVertex(x1, y2, z2, 0, 0, 0, 0, 0),
    });

    std::vector<u32> indices = std::vector<u32>({
        //front
        0, 1, 2,
        0, 2, 3,
        //right
        3, 2, 4,
        3, 4, 5,
        //back
        4, 6, 5,
        4, 7, 6,
        //left
        7, 1, 6,
        1, 0, 6,
        //top
        1, 7, 2,
        7, 4, 2,
        //bottom
        6, 3, 5,
        6, 0, 3
    });

    return loadMemory(
        vertices,
        vertices.size(),
        indices,
        indices.size(),
        texture,
        mesh
    );
}

bool Mesh::loadMemory(
    std::vector<GVertex>& vertices,
    sz vertCount,
    std::vector<u32>& indices,
    sz indexCount,
    Texture* texture,
    Mesh*& mesh
)
{
    mesh = new Mesh;
    return mesh->initialize(
        vertices,
        vertCount,
        indices,
        indexCount,
        texture
    );
}

/*bool Mesh::loadObj(
    std::string path,
    std::vector<Mesh*>& meshes,
    GameManager* gameManager
)
{
    return loadObj(
        path,
        meshes,
        nullptr,
        gameManager
    );
}

bool Mesh::loadObj(
    std::string path,
    std::vector<Mesh*>& meshes,
    NavMesh* navMesh,
    GameManager* gameManager,
    bool flipX
)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs |  (flipX ? aiProcess_FlipWindingOrder : 0));
    if (scene == NULL || gameManager == NULL)
    {
        return false;
    }

    std::vector<Mesh> outMeshes = std::vector<Mesh>();

    auto root = scene->mRootNode;
    
    for (u32 i = 0; i < root->mNumChildren; i++)
    {
        auto child = root->mChildren[i];
    }

    // For each NODE in SCENE
    for (u32 i = 0; i < root->mNumChildren; i++)
    {
        auto xmFloat3Comparator = [](XMFLOAT3 a, XMFLOAT3 b) { return a.x == b.x && a.y == b.y && a.z == b.z; };

        std::vector<PxVec3> nodeVertices;
        std::unordered_set<PxVec3, HashPxVector3, EqualsPxVector3> uniqueNodeVertices;
        //std::vector<u32> nodeIndices = std::vector<u32>();
        //std::vector<PolygonVertexArray::PolygonFace> nodeFaces = std::vector<PolygonVertexArray::PolygonFace>();

        auto child = root->mChildren[i];

        u32 faceIndexOffset = 0;

        //For each MESH in NODE
        bool skip = false;
        for (u32 j = 0; j < child->mNumMeshes; j++)
        {
            std::vector<GVertex> meshVertices = std::vector<GVertex>();
            std::vector<u32> meshIndices = std::vector<u32>();

            auto mesh = scene->mMeshes[child->mMeshes[j]];

            if (scene->mMaterials[mesh->mMaterialIndex]->GetName() == aiString("__TB_empty"))
            {
                skip = true;
                break;
            }
            //For each VERTEX in MESH
            for (u32 k = 0; k < mesh->mNumVertices; k++)
            {
                f32 u = 0.0f, v = 0.0f;
                if (mesh->mTextureCoords[0])
                {
                    u = mesh->mTextureCoords[0][k].x;
                    v = mesh->mTextureCoords[0][k].y;
                }

                GVertex vertex = GVertex(
                    mesh->mVertices[k].x * (flipX ? -1.0f : 1.0f),
                    mesh->mVertices[k].y,
                    mesh->mVertices[k].z,
                    mesh->mNormals[k].x * (flipX ? -1.0f : 1.0f),
                    mesh->mNormals[k].y,
                    mesh->mNormals[k].z,
                    -u,
                    v
                );

                auto nodeVertex = PxVec3(mesh->mVertices[k].x * (flipX ? -1.0f : 1.0f), mesh->mVertices[k].y, mesh->mVertices[k].z);
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
                auto& face = mesh->mFaces[k];

                //For each INDEX in FACE
                for (u32 l = 0; l < face.mNumIndices; l++)
                {
                    meshIndices.push_back(face.mIndices[l]);
                   
                }
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

                    texture = Graphics.lazyLoadTexture(std::string(str.C_Str()));
                }
            }

            // Create MESH for Rendering
            Mesh* _mesh = new Mesh();
            _mesh->initialize(
                meshVertices,
                meshVertices.size(),
                meshIndices,
                meshIndices.size(),
                texture
            );
            meshes.push_back(_mesh);
        }
        if (skip)
        {
            continue;
        }

        PxVec3* convexVertices = new PxVec3[nodeVertices.size()];
        CopyMemory(convexVertices, nodeVertices.data(), nodeVertices.size() * sizeof(PxVec3));

        PxConvexMeshDesc meshDesc;
        meshDesc.points.count = (u32)nodeVertices.size();
        meshDesc.points.stride = sizeof(PxVec3);
        meshDesc.points.data = convexVertices;
        meshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

        PxDefaultMemoryOutputStream buffer;
        PxConvexMeshCookingResult::Enum result;
        if (!gameManager->getCooking()->cookConvexMesh(meshDesc, buffer, &result))
        {
            logger.err("Failed to cook convex mesh!");
            return false;
        }
        PxDefaultMemoryInputData input(buffer.getData(), buffer.getSize());
        auto mesh = gameManager->getPhysics()->createConvexMesh(input);
        auto actor = gameManager->getPhysics()->createRigidStatic(PxTransform(PxVec3(0, 0, 0)));
        auto material = gameManager->getPhysics()->createMaterial(0.5f, 0.5f, 0.6f);

        auto shape = PxRigidActorExt::createExclusiveShape(*actor, PxConvexMeshGeometry(mesh), *material);

        gameManager->getPxScene()->addActor(*actor);
    }    

    if (navMesh)
    {
        GenerateNavMesh(&Game, navMesh, meshes);
    }

    return true;

}*/

/*bool Mesh::loadGltf(
    std::string path,
    Mesh** mesh,
    GameManager* gameManager
)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (scene == NULL || gameManager == NULL || scene->mNumMeshes < 1)
    {
        return false;
    }

    //For each MESH in SCENE
    bool skip = false;
    std::vector<GVertex> meshVertices = std::vector<GVertex>();
    std::vector<u32> meshIndices = std::vector<u32>();

    auto _mesh = scene->mMeshes[0];
  
    //For each VERTEX in MESH
    for (u32 i = 0; i < _mesh->mNumVertices; i++)
    {
        f32 u = 0.0f, v = 0.0f;
        if (_mesh->mTextureCoords[0])
        {
            u = _mesh->mTextureCoords[0][i].x;
            v = _mesh->mTextureCoords[0][i].y;
        }

        GVertex vertex = GVertex(
            _mesh->mVertices[i].x,
            _mesh->mVertices[i].y,
            _mesh->mVertices[i].z,
            _mesh->mNormals[i].x,
            _mesh->mNormals[i].y,
            _mesh->mNormals[i].z,
            u,
            v
        );
        
        meshVertices.push_back(vertex);
    }

    //For each FACE in MESH
    for (u32 i = 0; i < _mesh->mNumFaces; i++)
    {
        auto& face = _mesh->mFaces[i];

        //For each INDEX in FACE
        for (u32 j = 0; j < face.mNumIndices; j++)
        {
            meshIndices.push_back(face.mIndices[j]);
        }
    }

    //Get texture from material list on mesh
    Texture* texture = NULL;
    int texIdx = 0;
    if (_mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[_mesh->mMaterialIndex];
        logger.info("\t%s", material->GetName().C_Str());
        auto diffuseCount = material->GetTextureCount(aiTextureType_DIFFUSE);
        if (diffuseCount > 0)
        {
            aiString str;
            material->GetTexture(aiTextureType_DIFFUSE, 0, &str);

            if (auto _texture = scene->GetEmbeddedTexture(str.C_Str()))
            {
                logger.info("Embedded! %dx%d", _texture->mWidth, _texture->mHeight);
                texture = new Texture((u8*)_texture->pcData, _texture->mWidth);
            }
            else
            {
                texture = Graphics.lazyLoadTexture(std::string(str.C_Str()));
            }
        }
    }

    // Create MESH for Rendering
    *mesh = new Mesh();

    getBonesForMesh(*mesh, meshVertices, _mesh, scene);

    (*mesh)->initialize(
        meshVertices,
        meshVertices.size(),
        meshIndices,
        meshIndices.size(),
        texture
    );
    (*mesh)->setScene((aiScene*)scene);
    return true;
}*/

/*void Mesh::getBonesForMesh(Mesh* mesh, std::vector<GVertex>& vertices, aiMesh* aMesh, const aiScene* scene)
{
    for (u32 boneIndex = 0; boneIndex < aMesh->mNumBones; boneIndex++)
    {
        int boneId = -1;
        std::string boneName = aMesh->mBones[boneIndex]->mName.C_Str();

        if (mesh->getBoneInfoMap().find(boneName) == mesh->getBoneInfoMap().end())
        {
            BoneInfo newBone = { 0 };
            newBone.id = mesh->getBoneCounter();
            convertAiMatrixToXMMatrix(aMesh->mBones[boneIndex]->mOffsetMatrix, newBone.offset);
            mesh->getBoneInfoMap()[boneName] = newBone;
            boneId = mesh->getBoneCounter();
            mesh->setBoneCounter(mesh->getBoneCounter() + 1);
        }
        else
        {
            boneId = mesh->getBoneInfoMap()[boneName].id;
        }
        if (boneId == -1)
        {
            logger.err("Bone ID is -1, this is bullshit!");
            return;
        }
        auto weights = aMesh->mBones[boneIndex]->mWeights;
        i32 numWeights = aMesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; weightIndex++)
        {
            i32 vertexId = weights[weightIndex].mVertexId;
            f32 weight = weights[weightIndex].mWeight;
            if (vertexId >= vertices.size())
            {
                logger.err("Somehow, you've got more vertices in the assimp bone hierarchy than in the parsed-out vertex list. Fix yo shit!");
                return;
            }
            pushBoneToVertexStack(vertices[vertexId], boneId, weight);
        }
    }
}
*/
void Mesh::draw()
{
    if (m_texture != NULL)
    {
        m_texture->use();
    }

    UINT stride = sizeof(GVertex);
    UINT offset = 0;

    auto context = Graphics.getContext();

    context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context->DrawIndexed(m_indexCount, 0, 0);
}

Mesh::Mesh():
    m_indexBuffer(nullptr),
    m_vertexBuffer(nullptr),
    m_indexCount(0),
    m_scene(nullptr),
    m_status(true),
    m_texture(nullptr),
    m_textureCount(0),
    m_vertCount(0)
{

}

Mesh::Mesh(AABB aabb) :
    m_box(aabb),
    m_indexCount(0),
    m_scene(nullptr),
    m_status(true),
    m_texture(nullptr),
    m_textureCount(0),
    m_vertCount(0)
{

}

Mesh::Mesh(
    const std::vector<GVertex>& vertices,
    sz vertCount,
    const std::vector<u32>& indices,
    sz indexCount,
    Texture* texture
)
{
    initialize(vertices,
        vertCount,
        indices,
        indexCount,
        texture
    );
}

Mesh::~Mesh()
{

}

AABB Mesh::getBox()
{
    return m_box;
}

void Mesh::addLight(ILight* light)
{
    m_lights.push_back(light);
}

void Mesh::getTransforms(const std::string& name, f32 timePos, std::vector<XMMATRIX>& transforms)
{
    /*u32 numBones = m_boneOffsets.size();
    std::vector<XMMATRIX> toParentTransforms(numBones);

    auto clip = m_animations.find(name);
    clip->second.interpolate(timePos, toParentTransforms);

    std::vector<XMMATRIX> toRootTransforms(numBones);

    toRootTransforms[0] = toParentTransforms[0];

    for (u32 i = 1; i < numBones; i++)
    {
        XMMATRIX toParent = toParentTransforms[i];
        int parentIndex = m_boneHierarchy[i];

        XMMATRIX parentToRoot = toRootTransforms[parentIndex];
        XMMATRIX toRoot = XMMatrixMultiply(toParent, parentToRoot);

        toRootTransforms[i] = toRoot;
    }

    for (u32 i = 0; i < numBones; i++)
    {
        XMMATRIX offset = m_boneOffsets[i];
        XMMATRIX toRoot = toRootTransforms[i];
        transforms[i] = XMMatrixMultiply(offset, toRoot);
    }*/
}

Texture* Mesh::getTexture()
{
    return m_texture;
}

void Mesh::setTexture(Texture* texture)
{
    m_texture = texture;
}

void Mesh::setScene(aiScene* scene)
{
    m_scene = scene;
}

const aiScene* Mesh::getScene()
{
    return m_scene;
}


bool Mesh::initialize(
    const std::vector<GVertex>& vertices,
    sz vertCount,
    const std::vector<u32>& indices,
    sz indexCount,
    Texture* texture
)
{
    m_indices = indices;
    m_vertices = vertices;
    m_texture = texture;

    const GVertex* pVertices = vertices.data();
    const u32* pIndices = indices.data();

    m_vertCount = (u32)vertCount;
    m_indexCount = (u32)indexCount;

    D3D11_BUFFER_DESC vertexDesc = { 0 };
    vertexDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexDesc.ByteWidth = (UINT)(vertCount * sizeof(GVertex));
    vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexDesc.CPUAccessFlags = 0;
    vertexDesc.MiscFlags = 0;
    vertexDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexData = { 0 };
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

    D3D11_BUFFER_DESC indexDesc = { 0 };
    indexDesc.Usage = D3D11_USAGE_DEFAULT;
    indexDesc.ByteWidth = (u32)(sizeof(u32) * indexCount);
    indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexDesc.CPUAccessFlags = 0;
    indexDesc.MiscFlags = 0;
    indexDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA indexData { 0 };
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

    initAABB(vertices);
    return true;
}

XMFLOAT3 Mesh::getCentroid()
{
    GVertex centroidV = m_vertices[0];
    for (int i = 1; i < m_vertices.size(); i++)
    {
        centroidV.pos.x += m_vertices[i].pos.x;
        centroidV.pos.y += m_vertices[i].pos.y;
        centroidV.pos.z += m_vertices[i].pos.z;
    }
    centroidV.pos.x /= (float)m_vertices.size();
    centroidV.pos.y /= (float)m_vertices.size();
    centroidV.pos.z /= (float)m_vertices.size();

    return centroidV.pos;
}

/*std::map<std::string, BoneInfo>& Mesh::getBoneInfoMap()
{
    return m_boneInfoMap;
}

i32 Mesh::getBoneCounter()
{
    return m_boneCounter;
}

void Mesh::setBoneCounter(i32 boneCounter)
{
    m_boneCounter = boneCounter;
}*/

void Mesh::initAABB(const std::vector<GVertex>& vertices)
{
    m_box = AABB(vertices);
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
    sz initialSize = out.size();
    for (u32 i = 0; i < a.size(); i++)
    {
        out.push_back(a[i] + (u32)initialSize);
    }
    initialSize = out.size();
    for (u32 i = 0; i < b.size(); i++)
    {
        out.push_back(b[i] + (u32)initialSize);
    }
}

float sign(XMFLOAT2 p1, XMFLOAT2 p2, XMFLOAT2 p3)
{
    return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool PointInTriangle(XMFLOAT2 pt, XMFLOAT2 v1, XMFLOAT2 v2, XMFLOAT2 v3)
{
    float d1, d2, d3;
    bool has_neg, has_pos;

    d1 = sign(pt, v1, v2);
    d2 = sign(pt, v2, v3);
    d3 = sign(pt, v3, v1);

    has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    return !(has_neg && has_pos);
}

bool Mesh::GenerateNavMesh(GameManager* game, NavMesh* navMesh, std::vector<Mesh*> meshes)
{
    const static f32 INTERVAL = 64.0f;
    XMFLOAT3 _up = XMFLOAT3(0, 1.0f, 0);
    XMVECTOR up = XMLoadFloat3(&_up);

    auto scene = game->getPxScene();

    for (const auto& mesh : meshes)
    {
        for (u32 i = 0; i < mesh->m_indexCount; i += 3)
        {
            XMVECTOR normal = XMLoadFloat3(&mesh->m_vertices[mesh->m_indices[i]].normal);
            f32 cosTheta = XMVectorGetX(XMVector3Dot(normal, up));
            if (cosTheta >= 0.7f)
            {
                std::vector<GVertex> triangle({
                    mesh->m_vertices[mesh->m_indices[i]],
                    mesh->m_vertices[mesh->m_indices[i + 1]],
                    mesh->m_vertices[mesh->m_indices[i + 2]]
                });

                auto bbox = AABB(triangle);

                for (int x = ((i32)(bbox.getMin().x / INTERVAL)) * (int)INTERVAL; x < ((i32)(bbox.getMax().x / INTERVAL)) * (int)INTERVAL; x += (int)INTERVAL)
                {
                    for (int z = ((i32)(bbox.getMin().z / INTERVAL)) * (int)INTERVAL; z < ((i32)(bbox.getMax().z / INTERVAL)) * (int)INTERVAL; z += (int)INTERVAL)
                    {
                        auto origin = PxVec3((f32)x, bbox.getMax().y + 16.0f, (f32)z);
                        XMFLOAT2 _origin = XMFLOAT2(origin.x, origin.z);

                        if (PointInTriangle(
                            _origin,
                            XMFLOAT2(triangle[0].pos.x, triangle[0].pos.z),
                            XMFLOAT2(triangle[1].pos.x, triangle[1].pos.z),
                            XMFLOAT2(triangle[2].pos.x, triangle[2].pos.z)
                        ))
                        {
                            continue;
                        }

                        auto dir = PxVec3(0.0f, -1.0f, 0.0f);
                        PxReal dist = 1000.0f;
                        PxRaycastBuffer hit;

                        XMFLOAT3 pos(0.0f, 0.0f, 0.0f);

                        if (scene->raycast(origin, dir, dist, hit))
                        {
                            if (hit.block.position.y == bbox.getMax().y + 16.0f)
                            {
                                continue;
                            }
                            pos.x = hit.block.position.x;
                            pos.y = hit.block.position.y;
                            pos.z = hit.block.position.z;
                            navMesh->addNode(pos);
                        }

                    }
                }
            }
        }
    }

    return true;
}

/*void pushBoneToVertexStack(GVertex& vertex, i32 boneId, f32 weight)
{
    for (int i = 0; i < 4; i++)
    {
        auto boneArr = (i32*)&(vertex.bones); // is this safe? I think it's ok...
        if (boneArr[i] < 0)
        {
            auto weightArr = (f32*)&(vertex.weights);
            
            boneArr[i] = boneId;
            weightArr[i] = weight;
            return;
        }
    }
}*/