#include "Vertex.hlsl"

#include "Perspective.hlsl"

#include "Pixel.hlsl"

cbuffer Sprite : register(b1)
{
    float4 pos;
    float4 color;
    float2 scale;
};

PixelInput Vertex(VertexInput input)
{
    PixelInput output;
    output.normal = input.normal;
    output.tex = input.tex;
    
    float4 newPos = float4(input.position.xy * scale, input.position.zw);
    float3 right = float3(view[0][0], view[1][0], view[2][0]);
    float3 up = float3(0, 1.0f, 0);
    
    newPos = float4(pos.xyz
        + right * newPos.x
        + up * newPos.y,
        1.0f
    );
    
    output.pixelPos = newPos.xyz;
    
    output.position = PerspectiveTransform(
		newPos,
		world,
		view,
		projection
	);

    return output;
}