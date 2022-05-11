#include "Vertex.hlsl"

#include "Perspective.hlsl"
//#include "ShadowMap.hlsl"
#include "PointLight.hlsli"

PixelInput Vertex(VertexInput input)
{
    PixelInput output;
    output.pixelPos = float4(input.position.xyz, 1.0);
    output.normal = input.normal;
    output.tex = input.tex;
    
    output.position = PerspectiveTransform(
		input.position,
		world,
		view,
		projection
	);

    return output;
}