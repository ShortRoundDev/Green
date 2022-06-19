#include "Octree.h"

#include "MeshEntity.h"

#include <set>

Octree::Octree(const std::vector<MeshEntity*>& meshes) :
    m_root(nullptr)
{
    if (meshes.size() == 0)
    {
        return;
    }
    XMFLOAT3 min = meshes[0]->getMesh()->getBox().getMin();
    XMFLOAT3 max = meshes[0]->getMesh()->getBox().getMax();

    for (auto mesh : meshes)
    {
        auto bMax = mesh->getMesh()->getBox().getMax();
        auto bMin = mesh->getMesh()->getBox().getMin();
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

void Octree::insert(MeshEntity* meshEntity)
{
    m_root->insert(meshEntity);
}

void Octree::queryAll(AABB* aabb, std::vector<MeshEntity*>& meshes)
{
    queryType(aabb, meshes, 0);
}

void Octree::querySolid(AABB* aabb, std::vector<MeshEntity*>& meshes)
{
    queryType(aabb, meshes, WORLDSPAWN);
}

void Octree::queryType(AABB* aabb, std::vector<MeshEntity*>& meshes, u64 type)
{
    std::set<MeshEntity*> uniqueness;
    m_root->query(aabb, meshes, uniqueness, type);
}

void Octree::queryAll(XMFLOAT3 point, std::vector<MeshEntity*>& meshes)
{
    queryType(point, meshes, 0);
}

void Octree::querySolid(XMFLOAT3 point, std::vector<MeshEntity*>& meshes)
{
    queryType(point, meshes, WORLDSPAWN);
}

void Octree::queryType(XMFLOAT3 point, std::vector<MeshEntity*>& meshes, u64 type)
{
    std::set<MeshEntity*> uniqueness;
    m_root->query(point, meshes, uniqueness, type);
}
