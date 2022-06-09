#ifndef __GLOBAL_CBUFFER_HLSLI__
#define __GLOBAL_CBUFFER_HLSLI__

#include "DirectionalLight.hlsli"

cbuffer GlobalCBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix invWorld;
    float4 camera;
    
    DirectionalLight sun;
    
    float pointradius;
}

#endif