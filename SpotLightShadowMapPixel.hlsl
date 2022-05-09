#include "ShadowMap.hlsl"
#include "ShadowPixelInput.hlsl"

struct PS_OUT
{
    float4 color : SV_Target;
    float depth : SV_Depth;
};

PS_OUT Pixel(ShadowPixelInput input) : SV_TARGET
{
    PS_OUT output;
    output.color = float4(1, 1, 1, 1);
    output.depth = 1.0f;
    
    return output;
}