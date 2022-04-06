#ifndef __PIXEL_INPUT_HLSLI__
#define __PIXEL_INPUT_HLSLI__

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

#endif