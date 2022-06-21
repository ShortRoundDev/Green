#include "Mesh.h"

#include "GraphicsManager.h"
#include "GameManager.h"
#include "Logger.h"
#include "Texture.h"
#include "ILight.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "MeshViewModel.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <unordered_set>

static ::Logger logger = CreateLogger("Mesh");

struct HashPxVector3
{
public:
    size_t operator()(const PxVec3& vector) const
    {
        return (std::bit_cast<u32>(vector.x) * 73856093) ^
               (std::bit_cast<u32>(vector.y) * 19349663) ^
               (std::bit_cast<u32>(vector.z) * 83492791);
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

bool Mesh::createMapFromFile(
    std::string path,
    std::vector<Mesh*>& meshes,
    GameManager* gameManager
)
{
    return createMapFromFile(
        path, meshes, gameManager, false
    );
}

bool Mesh::createMapFromFile(
    std::string path,
    std::vector<Mesh*>& meshes,
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
        logger.info("%d: Meshes: %d", i, child->mNumMeshes);
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
        logger.info("Node: %s", child->mName.C_Str());
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
                auto face = mesh->mFaces[k];

                //For each INDEX in FACE
                //PolygonVertexArray::PolygonFace nodeFace = PolygonVertexArray::PolygonFace();
                //nodeFace.indexBase = faceIndexOffset;
                //nodeFace.nbVertices = face.mNumIndices;
                //nodeFaces.push_back(nodeFace);
                for (u32 l = 0; l < face.mNumIndices; l++)
                {
                    meshIndices.push_back(face.mIndices[l]);

                    //remap node index
                   // auto uniquePos = meshVertices[face.mIndices[l]];
                    //auto vertexIterator = std::find_if(nodeVertices.begin(), nodeVertices.end(), [uniquePos](const XMFLOAT3& a) {
                    //    return a.x == uniquePos.pos.x && a.y == uniquePos.pos.y && a.z == uniquePos.pos.z;
                    //});
                    //auto index = std::distance(nodeVertices.begin(), vertexIterator);
                    //nodeIndices.push_back(index);
                }
                //faceIndexOffset = nodeIndices.size();
            }

            //Get texture from material list on mesh
            Texture* texture = NULL;
            int texIdx = 0;
            if (mesh->mMaterialIndex >= 0)
            {
                aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
                logger.info("\t%s", material->GetName().C_Str());
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
        meshDesc.points.count = nodeVertices.size();
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
    Skip:
        continue;
        //physicsMeshes.push_back(gameManager->getPhysics()->createConvexMesh(input));
        


        //PxRigidStatic* st = gameManager->getPhysics()->createRigidStatic(PxTransform(PxVec3(0, 0, 0));
        //PxRigidActorExt::createExclusiveShape(*st, PxConvexMeshGeometry(mesh), nullptr);
        

        //physicsMeshes.push_back(new btConvexHullShape((btScalar*)vertices, nodeVertices.size(), sizeof(btVector3)));
        //PolyhedronMesh* nodeMesh = gameManager->getPhysicsCommon()->createPolyhedronMesh(nodeVertexArray);
        //physicsMeshes.push_back(gameManager->getPhysicsCommon()->createConvexMeshShape(nodeMesh));
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

bool Mesh::loadGltf(
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

    *mesh = new Mesh;

    auto root = scene->mRootNode;
    auto node = scene->mRootNode;

    //std::vector<PxVec3> nodeVertices;
    //std::unordered_set<PxVec3, HashPxVector3, EqualsPxVector3> uniqueNodeVertices;

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
        auto face = _mesh->mFaces[i];

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
    (*mesh)->initialize(
        meshVertices,
        meshVertices.size(),
        meshIndices,
        meshIndices.size(),
        texture
    );
    return true;

    /*PxVec3* convexVertices = new PxVec3[nodeVertices.size()];
    CopyMemory(convexVertices, nodeVertices.data(), nodeVertices.size() * sizeof(PxVec3));

    PxConvexMeshDesc meshDesc;
    meshDesc.points.count = nodeVertices.size();
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

    gameManager->getPxScene()->addActor(*actor);*/
}

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

Mesh::Mesh()
{

}

Mesh::Mesh(AABB aabb)
    : m_box(aabb)
{

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

MeshViewModel* Mesh::getViewModel()
{
    auto centroid = getCentroid();
    auto centroidV = XMLoadFloat3(&centroid);

    std::vector<ILight*> spotLights;
    std::vector<ILight*> pointLights;

    std::copy_if(m_lights.begin(), m_lights.end(), std::back_inserter(spotLights), [](ILight* val) {
        return val->getLightType() == SPOT_LIGHT;
    });

    std::copy_if(m_lights.begin(), m_lights.end(), std::back_inserter(pointLights), [](ILight* val) {
        return val->getLightType() == POINT_LIGHT;
    });
    if (spotLights.size() > MAX_LIGHT)
    {
        std::sort(spotLights.begin(), spotLights.end(), [centroidV](ILight* a, ILight* b) {
            auto aPos = a->getPos();
            XMVECTOR aV = XMLoadFloat4(&aPos);

            auto bPos = b->getPos();
            XMVECTOR bV = XMLoadFloat4(&bPos);

            XMVECTOR aDiff = XMVectorSubtract(centroidV, aV);
            XMVECTOR bDiff = XMVectorSubtract(centroidV, bV);

            auto aLengthV = XMVector3LengthSq(aDiff);
            auto bLengthV = XMVector3LengthSq(bDiff);

            f32 aLength, bLength;
            XMStoreFloat(&aLength, aLengthV);
            XMStoreFloat(&bLength, bLengthV);

            return aLength > bLength;
        });
        if (spotLights.size() > MAX_LIGHT)
        {
            logger.warn("Spot Light limit threshold exceeded!");
        }
        spotLights.erase(spotLights.begin() + 3, spotLights.end());
    }

    if (pointLights.size() > MAX_LIGHT)
    {
        std::sort(pointLights.begin(), pointLights.end(), [centroidV](ILight* a, ILight* b) {
            auto aPos = a->getPos();
            XMVECTOR aV = XMLoadFloat4(&aPos);

            auto bPos = b->getPos();
            XMVECTOR bV = XMLoadFloat4(&bPos);

            XMVECTOR aDiff = XMVectorSubtract(centroidV, aV);
            XMVECTOR bDiff = XMVectorSubtract(centroidV, bV);

            auto aLengthV = XMVector3LengthSq(aDiff);
            auto bLengthV = XMVector3LengthSq(bDiff);

            f32 aLength, bLength;
            XMStoreFloat(&aLength, aLengthV);
            XMStoreFloat(&bLength, bLengthV);

            return aLength > bLength;
        });
        if (pointLights.size() > MAX_LIGHT)
        {
            logger.warn("Point Light limit threshold exceeded!");
        }
        pointLights.erase(pointLights.begin() + 3, pointLights.end());
    }

    std::vector<SpotLight*> spotLightsSorted;
    std::vector<PointLight*> pointLightsSorted;

    std::transform(spotLights.begin(), spotLights.end(), std::back_inserter(spotLightsSorted), [](ILight* light) {
        return (SpotLight*)light;
    });

    std::transform(pointLights.begin(), pointLights.end(), std::back_inserter(pointLightsSorted), [](ILight* light) {
        return (PointLight*)light;
    });

    return new MeshViewModel(this, spotLightsSorted, pointLightsSorted);
}

void Mesh::getTransforms(const std::string& name, f32 timePos, std::vector<XMMATRIX>& transforms)
{
    u32 numBones = m_boneOffsets.size();
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
    }
}

Texture* Mesh::getTexture()
{
    return m_texture;
}

void Mesh::setTexture(Texture* texture)
{
    m_texture = texture;
}

bool Mesh::initialize(
    const std::vector<GVertex>& vertices,
    size_t vertCount,
    const std::vector<u32>& indices,
    size_t indexCount,
    Texture* texture
)
{
    m_vertices = vertices;
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