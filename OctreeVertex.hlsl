#include "Vertex.hlsl"
#include "Perspective.hlsl"
#include "PointLight.hlsli"

PixelInput Vertex(VertexInput input)
{
    PixelInput output;
    
    output.position = float4(input.position.xyz, 1.0f);
    output.pixelPos = output.position;
    
    output.normal = input.normal;
    output.tex = input.tex;
    output.position = PerspectiveTransform(
        output.position,
        world,
        view,
        projection
    );

    return output;
}