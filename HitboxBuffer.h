#pragma once

#include <DirectXMath.h>
using namespace DirectX;

struct HitboxBuffer
{
    XMFLOAT3 pos;
    f32 radius;
    XMFLOAT4 color;
};