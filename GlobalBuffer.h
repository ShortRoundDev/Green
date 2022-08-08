#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct SunBuffer
{
    XMFLOAT4 color;
    XMFLOAT4 direction;
    XMFLOAT4 pos;
    XMMATRIX dirLightSpace;
};

struct DirectionalLightBuffer
{
    //ambient
    XMFLOAT4 ambientA;
    XMFLOAT4 ambientB;
    XMFLOAT4 ambientDirection;

    //sun
    SunBuffer sun;

    //also ambient but I don't wanna pack this shit
    float hardness;
};

struct GlobalBuffer
{
    XMMATRIX world;
    XMMATRIX view;
    XMMATRIX projection;
    XMMATRIX invWorld;
    XMFLOAT4 camera;
    DirectionalLightBuffer dirLight;

    float pointradius;
};

struct ModelBuffer
{
    XMMATRIX modelTransform;
    XMMATRIX bones[100];
    u32 totalBones;
};