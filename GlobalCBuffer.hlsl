#ifndef __GLOBAL_CBUFFER_HLSLI__
#define __GLOBAL_CBUFFER_HLSLI__

#include "DirectionalLight.hlsli"

#define MAX_BONES 100

cbuffer GlobalCBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix invWorld;
    float4 camera;
    
    DirectionalLight dirLight;
    
    float pointradius;
}

cbuffer ModelBuffer : register(b1)
{
    matrix modelTransform;
    matrix bones[MAX_BONES];
    uint totalBones;
}

#endif