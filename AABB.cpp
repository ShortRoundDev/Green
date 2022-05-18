#include "AABB.h"

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
    m_max = max.pos;
}

AABB::~AABB()
{

}

const XMFLOAT3& AABB::getMin()
{
    return m_min;
}

const XMFLOAT3& AABB::getMax()
{
    return m_max;
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