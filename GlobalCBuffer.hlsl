#ifndef __GLOBAL_CBUFFER_HLSLI__
#define __GLOBAL_CBUFFER_HLSLI__

cbuffer GlobalCBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    matrix invWorld;
    float4 camera;
    float4 ambientLightColor;
    float4 lightDirection;
}

#endif