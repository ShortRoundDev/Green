#include "Vertex.hlsl"
#include "Perspective.hlsl"
#include "HitboxBuffer.hlsli"

PixelInput Vertex(VertexInput input)
{
    PixelInput output;

    output.tex = input.tex;
    output.pixelPos = input.position.xyz;
    output.position = PerspectiveTransform(
        float4(input.position + pos, 1.0f),
        world,
        view,
        projection
    );

    return output;
}