#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct GVertex
{
    XMFLOAT3 pos;    //  0 - 11
    XMFLOAT3 normal; // 12 - 23
    XMFLOAT2 tex;    // 24 - 31
    XMINT4 bones;
    XMFLOAT4 weights;

    GVertex()
    {

    }

    GVertex(float x, float y, float z, float nX, float nY, float nZ, float u, float v, XMINT4 _bones = { -1, -1, -1, -1}, XMFLOAT4 _weights = { 0.0f, 0.0f, 0.0f, 0.0f })
    {
        pos = XMFLOAT3(x, y, z);
        normal = XMFLOAT3(nX, nY, nZ);
        tex = XMFLOAT2(u, v);
        bones = _bones;
        weights = _weights;
    }
};
