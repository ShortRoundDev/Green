#include "Pixel.hlsl"

Texture2D albedo : register(t0);
SamplerState sampleType : register(s0);

float4 Pixel(PixelInput input) : SV_TARGET
{
    float4 color = albedo.Sample(sampleType, input.tex);
    if (color.a == 0.0f)
    {
        discard;
    }
    return color;
}