
#include "AABB.h"

#include "GTypes.h"

AABB::AABB() :
    AABB({0, 0, 0}, {0, 0, 0})
{

}

AABB::AABB(const XMFLOAT3& min, const XMFLOAT3& max)
{
    m_min = min;
    m_max = max;
}

AABB::AABB(const std::vector<GVertex>& vertices)
{
    GVertex min = vertices[0];
    GVertex max = vertices[0];
    for (const auto& vertex : vertices)
    {
        if (vertex.pos.x < min.pos.x)
        {
            min.pos.x = vertex.pos.x;
        }
        if (vertex.pos.y < min.pos.y)
        {
            min.pos.y = vertex.pos.y;
        }
        if (vertex.pos.z < min.pos.z)
        {
            min.pos.z = vertex.pos.z;
        }

        if (vertex.pos.x > max.pos.x)
        {
            max.pos.x = vertex.pos.x;
        }
        if (vertex.pos.y > max.pos.y)
        {
            max.pos.y = vertex.pos.y;
        }
        if (vertex.pos.z > max.pos.z)
        {
            max.pos.z = vertex.pos.z;
        }
    }
    m_min = min.pos;
    m_minV = XMLoadFloat3(&m_min);

    m_max = max.pos;
    m_maxV = XMLoadFloat3(&m_max);
    (
        static_cast<i32>(m_max.x),
        static_cast<i32>(m_max.y),
        static_cast<i32>(m_max.z)
    );
}

AABB::~AABB()
{

}

const XMFLOAT3& AABB::getMin()
{
    return m_min;
}

const XMVECTOR& AABB::getMinV()
{
    return m_minV;
}

const XMFLOAT3& AABB::getMax()
{
    return m_max;
}

const XMVECTOR& AABB::getMaxV()
{
    return m_maxV;
}

XMFLOAT3 AABB::getCentroid()
{
    XMVECTOR minV = XMLoadFloat3(&m_min);
    XMVECTOR maxV = XMLoadFloat3(&m_max);

    XMVECTOR sumV = XMVectorAdd(minV, maxV);
    XMVECTOR centroidV = XMVectorScale(sumV, 0.5f);

    XMFLOAT3 centroid;
    XMStoreFloat3(&centroid, centroidV);
    return centroid;
}


bool AABB::collides(AABB* other)
{
    const auto& bMax = other->getMax();
    const auto& bMin = other->getMin();

    return
        (m_min.x <= bMax.x && m_max.x >= bMin.x) &&
        (m_min.y <= bMax.y && m_max.y >= bMin.y) &&
        (m_min.z <= bMax.z && m_max.z >= bMin.z);
}

bool AABB::contains(XMFLOAT3 point)
{
    return  point.x >= m_min.x && point.x <= m_max.x &&
            point.y >= m_min.y && point.y <= m_max.y &&
            point.z >= m_min.z && point.z <= m_max.z;
}