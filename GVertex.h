#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct GVertex
{
    XMFLOAT3 pos;    //  0 - 11
    XMFLOAT3 normal; // 12 - 23
    XMFLOAT2 tex;    // 24 - 31

    GVertex(float x, float y, float z, float nX, float nY, float nZ, float u, float v)
    {
        pos = XMFLOAT3(x, y, z);
        normal = XMFLOAT3(nX, nY, nZ);
        tex = XMFLOAT2(u, v);
    }
};
