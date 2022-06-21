#include "Pixel.hlsl"

Texture2D albedo : register(t0);
SamplerState sampleType : register(s0);

cbuffer Sprite : register(b2)
{
    float4 pos;
    float4 color;
    float2 scale;
};

float4 Pixel(PixelInput input) : SV_TARGET
{
    float4 texColor = albedo.Sample(sampleType, input.tex) * color;
    if (texColor.a == 0.0f)
    {
        discard;
    }
    return texColor;
}