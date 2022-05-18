#include "Octree.h"

#include "Mesh.h"

#include <set>

Octree::Octree(const std::vector<Mesh*>& meshes) :
    m_root(nullptr)
{
    if (meshes.size() == 0)
    {
        return;
    }
    XMFLOAT3 min = meshes[0]->getBox().getMin();
    XMFLOAT3 max = meshes[0]->getBox().getMax();

    for (auto mesh : meshes)
    {
        auto bMax = mesh->getBox().getMax();
        auto bMin = mesh->getBox().getMin();
        if (bMin.x < min.x)
        {
            min.x = bMin.x;
        }
        if (bMin.y < min.y)
        {
            min.y = bMin.y;
        }
        if (bMin.z < min.z)
        {
            min.z = bMin.z;
        }

        if (bMax.x > max.x)
        {
            max.x = bMax.x;
        }
        if (bMax.y > max.y)
        {
            max.y = bMax.y;
        }
        if (bMax.z > max.z)
        {
            max.z = bMax.z;
        }
    }

    m_root = new OctreeNode(nullptr, AABB(min, max), meshes, 5); // max depth is 5. Can change later
}

Octree::~Octree()
{

}

void Octree::query(AABB* aabb, std::vector<Mesh*>& meshes)
{
    std::set<Mesh*> uniqueness;
    m_root->query(aabb, meshes, uniqueness);
}