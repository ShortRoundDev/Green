#pragma once

//Forward declarations

#include "OctreeNode.h"

#include <vector>

class MeshEntity;
class AABB;

class Octree
{
public:
    Octree(const std::vector<MeshEntity*>& meshes);
    ~Octree();

    void queryAll(AABB* aabb, std::vector<MeshEntity*>& meshes);
    void querySolid(AABB* aabb, std::vector<MeshEntity*>& meshes);
    void queryType(AABB* aabb, std::vector<MeshEntity*>& meshes, u64 type);

    void queryAll(XMFLOAT3 point, std::vector<MeshEntity*>& meshes);
    void querySolid(XMFLOAT3 point, std::vector<MeshEntity*>& meshes);
    void queryType(XMFLOAT3 point, std::vector<MeshEntity*>& meshes, u64 type);

    void insert(MeshEntity* meshEntity);

private:
    OctreeNode* m_root;
};
