#include "Vertex.hlsl"

#include "Perspective.hlsl"
#include "ShadowPixelInput.hlsl"
#include "ShadowMap.hlsl"

ShadowPixelInput Vertex(VertexInput input)
{
    ShadowPixelInput output;
    
    input.position.w = 1.0f;
   
    output.position = mul(input.position, lightSpace);
    output.tex = input.tex;

    return output;
}