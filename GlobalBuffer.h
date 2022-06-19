#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct DirectionalLight
{
    XMFLOAT4 ambientA;
    XMFLOAT4 ambientB;
    XMFLOAT4 ambientDirection;
    XMFLOAT4 color;
    XMFLOAT4 direction;
    float hardness;
};

struct GlobalBuffer
{
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
    XMMATRIX invWorld;
    XMFLOAT4 camera;
    DirectionalLight sun;

    float pointradius;
};