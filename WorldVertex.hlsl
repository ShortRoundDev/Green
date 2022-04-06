#include "Vertex.hlsl"

#include "Perspective.hlsl"

PixelInput Vertex(VertexInput input)
{
    PixelInput output;

    output.position = PerspectiveTransform(
		input.position,
		world,
		view,
		projection
	);
    output.tex = input.tex;

    return output;
}