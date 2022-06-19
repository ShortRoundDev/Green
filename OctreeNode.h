#pragma once

#include "AABB.h"
#include "GTypes.h"

#include <vector>
#include <set>

class MeshEntity;

class OctreeNode
{
public:
    OctreeNode(OctreeNode* parent, AABB bounds, const std::vector<MeshEntity*>& meshes, u8 division);
    ~OctreeNode();

    AABB getBounds();
    void query(AABB* aabb, std::vector<MeshEntity*>& meshes, std::set<MeshEntity*>& unique, u64 type);
    void query(XMFLOAT3 point, std::vector<MeshEntity*>& meshes, std::set<MeshEntity*>& unique, u64 type);
    void insert(MeshEntity* meshEntity);

private:
    AABB m_bounds;
    OctreeNode* m_parent;
    OctreeNode* m_children[8];
    u8 m_division;

    std::vector<MeshEntity*> m_meshes;

    void insert(MeshEntity* meshEntity, u8 division);
};

