#pragma once

#include "GTypes.h"
#include "AABB.h"

#include "DirectXMath.h"

using namespace DirectX;

class Frustum
{
public:
    Frustum();
    ~Frustum();

    void load(f32 depth, XMMATRIX projection, XMMATRIX view);
    XMVECTOR* getPlanes();

    bool checkPoint(const XMFLOAT3& point);
    bool checkBox(AABB* box);

private:
    XMVECTOR m_planes[6];
};

