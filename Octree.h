#pragma once

//Forward declarations

#include "OctreeNode.h"

#include <vector>

class Mesh;
class AABB;

class Octree
{
public:
    Octree(const std::vector<Mesh*>& meshes);
    ~Octree();

    void query(AABB* aabb, std::vector<Mesh*>& meshes);

private:
    OctreeNode* m_root;
};
