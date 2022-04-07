#include "Vertex.hlsl"

#include "Perspective.hlsl"
#include "ShadowPixelInput.hlsl"

cbuffer LightBuffer : register(b1)
{
    float4 color;
    matrix lightSpace;
};

ShadowPixelInput Vertex(VertexInput input)
{
    ShadowPixelInput output;
    
    input.position.w = 1.0f;
   
    output.position = mul(input.position, lightSpace);
    output.tex = input.tex;

    return output;
}