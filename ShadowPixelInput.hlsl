#ifndef __SHADOW_PIXEL_INPUT_HLSLI__
#define __SHADOW_PIXEL_INPUT_HLSLI__

struct ShadowPixelInput
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 lightViewPosition : TEXCOORD1;
    float3 lightPos : TEXCOORD2;
};

#endif