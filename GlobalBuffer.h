#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct GlobalBuffer
{
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
    XMMATRIX invWorld;
    XMFLOAT4 camera;
    XMFLOAT4 ambientLightColor;
    XMFLOAT4 lightDirection;
};