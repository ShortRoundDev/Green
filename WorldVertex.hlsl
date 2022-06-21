#include "Vertex.hlsl"

#include "Perspective.hlsl"
//#include "ShadowMap.hlsl"
#include "PointLight.hlsli"

PixelInput Vertex(VertexInput input)
{
    PixelInput output;
    output.position = float4(input.position.xyz, 1.0);
    output.pixelPos = mul(output.position, modelTransform);
    output.position = mul(output.position, modelTransform);
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