#ifndef __PIXEL_INPUT_HLSLI__
#define __PIXEL_INPUT_HLSLI__

struct PixelInput
{
    float4 position : SV_POSITION;
    float3 pixelPos: POSITION;
    float3 normal : NORMAL0;
    float2 tex : TEXCOORD0;
    float4 pixelPosLightSpace : POSITION1;
};

#endif