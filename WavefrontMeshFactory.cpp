#include "WavefrontMeshFactory.h"

#include "Texture.h"
#include "Mesh.h"
#include "GVertex.h"
#include "Logger.h"
#include "GraphicsManager.h"

#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <DirectXMath.h>
#include <PxPhysicsAPI.h>

#include <Windows.h>

#include <unordered_set>

using namespace DirectX;
using namespace physx;

static Logger logger = CreateLogger("WavefrontMeshFactory");

WavefrontMeshFactory::WavefrontMeshFactory(
    std::string path,
    bool flipX,
    GameManager* game
) :
    m_path(path),
    m_status(true),
    m_nodeIterator(0),
    m_meshIterator(0),
    m_flipX(flipX),
    m_game(game)
{
    HANDLE handle = CreateFileA(path.c_str(),
        FILE_READ_ACCESS,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if(handle == INVALID_HANDLE_VALUE)
    {
        m_status = false;
        return;
    }
    else
    {
        CloseHandle(handle);
    }
}

WavefrontMeshFactory::~WavefrontMeshFactory()
{

}

bool WavefrontMeshFactory::createMesh(MeshActor& meshActor)
{
    Assimp::Importer importer;
    auto scene = importer.ReadFile(
        m_path,
        aiProcess_Triangulate
        | aiProcess_FlipUVs
        | (m_flipX ? aiProcess_FlipWindingOrder : 0)
    );
    if (scene == NULL)
    {
        m_status = false;
        return false;
    }
    createMesh(meshActor, scene);
    return true;
}

bool WavefrontMeshFactory::createMesh(MeshActor& meshActor, const aiScene* scene)
{
    if (!m_status)
    {
        return false;
    }

    auto xmFloat3Comparator = [](XMFLOAT3 a, XMFLOAT3 b) {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    };

    std::vector<PxVec3> nodeVertices;
    std::unordered_set<PxVec3, HashPxVector3, EqualsPxVector3> uniqueNodeVertices;

    auto root = scene->mRootNode;

    if (!root || (m_nodeIterator >= root->mNumChildren))
    {
        m_status = false;
        return false;
    }

    auto child = root->mChildren[m_nodeIterator];

    if (m_meshIterator >= child->mNumMeshes)
    {
        //This could be an assert because this should never happen...
        m_nodeIterator++; // skip to next sibling
        m_meshIterator = 0;
        return false;
    }

    //check for non-solid meshes. These should not be drawn and so we skip them for the next sibling
    auto firstMesh = scene->mMeshes[child->mMeshes[0]];
    if (scene->mMaterials[firstMesh->mMaterialIndex]->GetName() == aiString("__TB_empty"))
    {
        m_meshIterator++;
        return false;
    }

    u32 faceIndexOffset = 0;
    //For each MESH in NODE
    std::vector<GVertex> meshVertices = std::vector<GVertex>();
    std::vector<u32>     meshIndices  = std::vector<u32>();

    auto mesh = scene->mMeshes[child->mMeshes[m_meshIterator]];

    //For each VERTEX in MESH
    for (u32 k = 0; k < mesh->mNumVertices; k++)
    {
        //Collect UV of vertex
        f32 u = 0.0f, v = 0.0f;
        if (mesh->mTextureCoords[0])
        {
            u = mesh->mTextureCoords[0][k].x;
            v = mesh->mTextureCoords[0][k].y;
        }

        GVertex vertex = GVertex(
            //position
            mesh->mVertices[k].x * (m_flipX ? -1.0f : 1.0f), // convert from stupid ass quake coordinates
            mesh->mVertices[k].y,
            mesh->mVertices[k].z,
            //normal vector
            mesh->mNormals[k].x * (m_flipX ? -1.0f : 1.0f),
            mesh->mNormals[k].y,
            mesh->mNormals[k].z,
            //texture
            -u,
            v
        );

        auto nodeVertex = PxVec3(
            mesh->mVertices[k].x * (m_flipX ? -1.0f : 1.0f), // convert from stupid ass quake coordinates
            mesh->mVertices[k].y,
            mesh->mVertices[k].z
        );

        // only need unique vertices for the the PhysX mesh. quake .map files store everything as
        // convex meshes, so all we need is the unique vertices in order to get the convex hull
        if (uniqueNodeVertices.find(nodeVertex) == uniqueNodeVertices.end()) // not found
        {
            uniqueNodeVertices.insert(nodeVertex); // using unordered set to determine uniqueness
            nodeVertices.push_back(nodeVertex); // nodeVertices hold the actual values though
        }

        //Vertices for drawing
        meshVertices.push_back(vertex);
    }

    //For each FACE in MESH
    for (u32 k = 0; k < mesh->mNumFaces; k++)
    {
        auto face = mesh->mFaces[k];

        //For each INDEX in FACE
        for (u32 l = 0; l < face.mNumIndices; l++)
        {
            //mesh indices for drawing
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
    Mesh* _mesh = new Mesh(
        meshVertices,
        meshVertices.size(),
        meshIndices,
        meshIndices.size(),
        texture
    );

    PxVec3* convexVertices = new PxVec3[nodeVertices.size()];
    CopyMemory(convexVertices, nodeVertices.data(), nodeVertices.size() * sizeof(PxVec3));

    PxConvexMeshDesc meshDesc;
    meshDesc.points.count = (u32)nodeVertices.size();
    meshDesc.points.stride = sizeof(PxVec3);
    meshDesc.points.data = convexVertices;
    meshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

    PxDefaultMemoryOutputStream buffer;
    PxConvexMeshCookingResult::Enum result;
    if (!m_game->getCooking()->cookConvexMesh(meshDesc, buffer, &result))
    {
        logger.err("Failed to cook convex mesh!");
        m_meshIterator++;
        return false;
    }
    PxDefaultMemoryInputData input(buffer.getData(), buffer.getSize());
    auto pxMesh = m_game->getPhysics()->createConvexMesh(input);
    auto actor = m_game->getPhysics()->createRigidStatic(PxTransform(PxVec3(0, 0, 0)));
    auto material = m_game->getPhysics()->createMaterial(0.5f, 0.5f, 0.6f);

    auto shape = PxRigidActorExt::createExclusiveShape(*actor, PxConvexMeshGeometry(pxMesh), *material);

    m_game->getPxScene()->addActor(*actor);

    meshActor.mesh = _mesh;
    meshActor.actor = actor;

    m_meshIterator++;

    return true;
}

void WavefrontMeshFactory::createMeshes(std::vector<MeshActor>& meshActors)
{
    Assimp::Importer importer;
    auto scene = importer.ReadFile(
        m_path,
        aiProcess_Triangulate
        | aiProcess_FlipUVs
        | (m_flipX ? aiProcess_FlipWindingOrder : 0)
    );
    if (scene == NULL)
    {
        m_status = false;
        return;
    }

    while (m_status)
    {
        MeshActor meshActor;
        if (createMesh(meshActor, scene))
        {
            meshActors.push_back(meshActor);
        }
    }
}

bool WavefrontMeshFactory::getStatus()
{
    return m_status;
}