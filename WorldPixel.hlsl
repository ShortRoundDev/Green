//#include "WorldObjectCBuffer.hlsli"
#include "Pixel.hlsl"
#include "Gamma.hlsl"

Texture2D albedo;
SamplerState sampleType;

float4 Pixel(PixelInput input) : SV_TARGET
{
    float4 texColor = albedo.Sample(sampleType, input.tex);
    //texColor = Gamma(texColor);

    return texColor;
}