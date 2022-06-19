#include "OctreeNode.h"

#include "MeshEntity.h"

OctreeNode::OctreeNode(OctreeNode* parent, AABB bounds, const std::vector<MeshEntity*>& meshes, u8 division) :
    m_parent(parent),
    m_bounds(bounds),
    m_division(division)
{
    memset(m_children, 0, sizeof(m_children));
    if (division == 0 || meshes.size() <= 1)
    {
        m_meshes = meshes;
        return;
    }

    XMVECTOR minV = XMLoadFloat3(&(bounds.getMin()));
    XMVECTOR maxV = XMLoadFloat3(&(bounds.getMax()));
    XMVECTOR diffV = XMVectorSubtract(maxV, minV);
    XMVECTOR halfSizeV = XMVectorScale(diffV, 0.5f);
    XMFLOAT3 halfSize;
    XMFLOAT3 half;
    XMVECTOR halfV = XMVectorAdd(minV, halfSizeV);

    XMStoreFloat3(&half, halfV);
    XMStoreFloat3(&halfSize, halfSizeV);

    XMFLOAT3 min = bounds.getMin();
    XMFLOAT3 max = bounds.getMax();

    AABB octants[8] = {
        AABB(min, half),
        AABB({ half.x, min.y,  min.z },   { max.x,  half.y, half.z }),
        AABB({ min.x,  half.y, min.z },   { half.x, max.y,  half.z }),
        AABB({ half.x, half.y, min.z },   { max.x,  max.y,  half.z }),

        AABB({ min.x,  min.y,  half.z },  { half.x, half.y, max.z  }),
        AABB({ half.x, min.y,  half.z },  { max.x,  half.y, max.z  }),
        AABB({ min.x,  half.y, half.z },  { half.x, max.y,  max.z  }),
        AABB(half, max)
    };

    std::vector<MeshEntity*> addToChildren[8];
    for (auto mesh : meshes)
    {
        for (int i = 0; i < 8; i++)
        {
            if (mesh->getMesh()->getBox().collides(&octants[i]))
            {
                addToChildren[i].push_back(mesh);
            }
        }
    }
    for (int i = 0; i < 8; i++)
    {
        m_children[i] = new OctreeNode(this, octants[i], addToChildren[i], division - 1);
    }
}

void OctreeNode::insert(MeshEntity* meshEntity)
{
    insert(meshEntity, 5);
}

void OctreeNode::insert(MeshEntity* meshEntity, u8 division)
{
    if (division == 0)
    {
        m_meshes.push_back(meshEntity);
        return;
    }

    XMVECTOR minV = XMLoadFloat3(&(m_bounds.getMin()));
    XMVECTOR maxV = XMLoadFloat3(&(m_bounds.getMax()));
    XMVECTOR diffV = XMVectorSubtract(maxV, minV);
    XMVECTOR halfSizeV = XMVectorScale(diffV, 0.5f);
    XMFLOAT3 halfSize;
    XMFLOAT3 half;
    XMVECTOR halfV = XMVectorAdd(minV, halfSizeV);

    XMStoreFloat3(&half, halfV);
    XMStoreFloat3(&halfSize, halfSizeV);

    XMFLOAT3 min = m_bounds.getMin();
    XMFLOAT3 max = m_bounds.getMax();

    AABB octants[8] = {
        AABB(min, half),
        AABB({ half.x, min.y,  min.z },   { max.x,  half.y, half.z }),
        AABB({ min.x,  half.y, min.z },   { half.x, max.y,  half.z }),
        AABB({ half.x, half.y, min.z },   { max.x,  max.y,  half.z }),

        AABB({ min.x,  min.y,  half.z },  { half.x, half.y, max.z  }),
        AABB({ half.x, min.y,  half.z },  { max.x,  half.y, max.z  }),
        AABB({ min.x,  half.y, half.z },  { half.x, max.y,  max.z  }),
        AABB(half, max)
    };

    MeshEntity* addToChildren[8];
    ZeroMemory(addToChildren, sizeof(addToChildren));
    for (int i = 0; i < 8; i++)
    {
        if (meshEntity->getMesh()->getBox().collides(&octants[i]))
        {
            addToChildren[i] = meshEntity;
        }
    }
    for (int i = 0; i < 8; i++)
    {
        if (addToChildren[i])
        {
            m_children[i]->insert(meshEntity, division - 1);
        }
    }
}

OctreeNode::~OctreeNode()
{

}

AABB OctreeNode::getBounds()
{
    return m_bounds;
}

void OctreeNode::query(AABB* aabb, std::vector<MeshEntity*>& meshes, std::set<MeshEntity*>& unique, u64 type)
{
    if (m_meshes.size() > 0 || m_children[0] == NULL) // leaf
    {
        for (auto mesh : m_meshes)
        {
            if (std::find(unique.begin(), unique.end(), mesh) != unique.end())
            {
                continue;
            }
            auto other = mesh->getMesh()->getBox();
            // type == 0 means all types
            if ((type == 0 || mesh->getType() == type) && other.collides(aabb) || aabb->collides(&other))
            {
                meshes.push_back(mesh);
                unique.emplace(mesh);
            }
        }
        return;
    }
    for (int i = 0; i < 8; i++)
    {
        if (m_children[i]->getBounds().collides(aabb))
        {
            m_children[i]->query(aabb, meshes, unique, type);
        }
    }
}

void OctreeNode::query(XMFLOAT3 point, std::vector<MeshEntity*>& meshes, std::set<MeshEntity*>& unique, u64 type)
{
    if (m_meshes.size() > 0 || m_children[0] == NULL) // leaf
    {
        for (auto mesh : m_meshes)
        {
            if (std::find(unique.begin(), unique.end(), mesh) != unique.end())
            {
                continue;
            }
            auto other = mesh->getMesh()->getBox();
            // type == 0 means all types
            if ((type == 0 || mesh->getType() == type) && other.contains(point))
            {
                meshes.push_back(mesh);
                unique.emplace(mesh);
            }
        }
        return;
    }
    for (int i = 0; i < 8; i++)
    {
        if (m_children[i]->getBounds().contains(point))
        {
            m_children[i]->query(point, meshes, unique, type);
        }
    }
}
