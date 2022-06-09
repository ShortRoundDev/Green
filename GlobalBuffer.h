#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct DirectionalLight
{
    XMFLOAT4 ambient;
    XMFLOAT4 color;
    XMFLOAT4 direction;
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