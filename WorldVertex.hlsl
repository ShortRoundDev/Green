#include "Vertex.hlsl"

#include "Perspective.hlsl"
//#include "ShadowMap.hlsl"
#include "PointLight.hlsli"

PixelInput Vertex(VertexInput input)
{
    PixelInput output;
    output.fragPos = float4(input.position.xyz, 1.0);
    output.normal = input.normal;
    output.tex = input.tex;
    
    float3 posToLight = input.position.xyz - lightPos.xyz;
    
    int axis = 0; // right
    float component = 0;
    float
        xDot = abs(dot(posToLight, float3(1, 0, 0))),
        yDot = abs(dot(posToLight, float3(0, 1, 0))),
        zDot = abs(dot(posToLight, float3(0, 0, 1)));
    if (xDot > yDot)
    {
        if (xDot > zDot)
        {
            axis = 0;
            component = posToLight.x;
        }
        else
        {
            axis = 2;
            component = posToLight.z;
        }
    }
    else
    {
        if (yDot > zDot)
        {
            axis = 1;
            component = posToLight.y;
        }
        else
        {
            axis = 2;
            component = posToLight.z;
        }
    }
    axis *= 2;
    if (component < 0)
    {
        axis += 1;
    }
    
    output.pixelPosLightSpace = mul(float4(output.fragPos, 1.0), lightSpace[0]);
    
    output.position = PerspectiveTransform(
		input.position,
		world,
		view,
		projection
	);

    return output;
}