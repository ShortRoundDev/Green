#include "Vertex.hlsl"

#include "Perspective.hlsl"
#include "ShadowPixelInput.hlsl"
#include "ShadowMap.hlsl"

ShadowPixelInput Vertex(VertexInput input)
{
    ShadowPixelInput output;
    output.position = mul(float4(input.position.xyz, 1.0), modelTransform);

   /* output.position = PerspectiveTransform(
		input.position,
		world,
		view,
		projection
	);*/
    output.position = mul(output.position, lightSpace);
    
    output.tex = input.tex;

    return output;
}