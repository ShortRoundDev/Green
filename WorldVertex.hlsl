#include "Vertex.hlsl"

#include "Perspective.hlsl"
#include "ShadowMap.hlsl"

PixelInput Vertex(VertexInput input)
{
    PixelInput output;
    output.fragPos = float4(input.position.xyz, 1.0);
    output.normal = input.normal;
    output.tex = input.tex;
    output.pixelPosLightSpace = mul(float4(output.fragPos, 1.0), lightSpace);
    
    output.position = PerspectiveTransform(
		input.position,
		world,
		view,
		projection
	);

    return output;
}