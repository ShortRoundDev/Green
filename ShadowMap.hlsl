#ifndef __SHADOW_MAP_HLSL__
#define __SHADOW_MAP_HLSL__

cbuffer LightBuffer : register(b1)
{
    matrix lightSpace;
    float4 color;
    float4 lightPos;
};

#endif
