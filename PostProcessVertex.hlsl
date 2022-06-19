#include "VertexInput.hlsl"

#include "GlobalCBuffer.hlsl"
#include "Perspective.hlsl"

struct VS_OUTPUT
{
    float4 Position : POSITION; // vertex position
    float2 TexCoord : TEXCOORD0; // vertex interpolation value
};

VS_OUTPUT Vertex(VertexInput input)
{
    VS_OUTPUT output;
    output.Position = PerspectiveTransform(input.position * 4096.0f, world, view, projection);
    output.TexCoord = input.tex;
    return output;
}