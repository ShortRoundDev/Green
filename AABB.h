#pragma once

#include "GVertex.h"

#include <DirectXMath.h>

#include <vector>

using namespace DirectX;

class AABB
{
public:
    AABB();
    AABB(const XMFLOAT3& min, const XMFLOAT3& max);
    AABB(const std::vector<GVertex>& vertices);
    ~AABB();

    const XMFLOAT3& getMin();
    const XMVECTOR& getMinV();
    const XMFLOAT3& getMax();
    const XMVECTOR& getMaxV();

    XMFLOAT3 getCentroid();

    bool collides(AABB* other);
    bool contains(XMFLOAT3 point);

private:
    XMFLOAT3 m_min;
    XMVECTOR m_minV;
    XMFLOAT3 m_max;
    XMVECTOR m_maxV;
};

