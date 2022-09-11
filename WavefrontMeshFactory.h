#pragma once
#include "IMeshFactory.h"
#include "GameManager.h"
#include "GTypes.h"

#include <assimp/scene.h>

#include <string>

struct HashPxVector3
{
public:
    sz operator()(const PxVec3& vector) const
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

class WavefrontMeshFactory : public IMeshFactory
{
public:
    WavefrontMeshFactory(std::string path, bool flipX = false, GameManager* game = &Game);
    ~WavefrontMeshFactory();
    bool createMesh(MeshActor& meshActor);
    void createMeshes(std::vector<MeshActor>& meshes);
    
    bool getStatus();

private:
    std::string m_path;
    bool m_status;
    u32 m_nodeIterator;
    u32 m_meshIterator;
    bool m_flipX;
    GameManager* m_game;

    bool createMesh(MeshActor& meshActor, const aiScene* scene);
};