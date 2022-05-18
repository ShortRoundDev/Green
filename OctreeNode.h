#pragma once

#include "AABB.h"
#include "GTypes.h"

#include <vector>
#include <set>

class Mesh;

class OctreeNode
{
public:
    OctreeNode(OctreeNode* parent, AABB bounds, const std::vector<Mesh*>& meshes, u8 division);
    ~OctreeNode();

    AABB getBounds();
    void query(AABB* aabb, std::vector<Mesh*>& meshes, std::set<Mesh*>& unique);

private:
    AABB m_bounds;
    OctreeNode* m_parent;
    OctreeNode* m_children[8];
    u8 m_division;

    std::vector<Mesh*> m_meshes;
};

