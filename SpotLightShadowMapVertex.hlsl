#include "Vertex.hlsl"

#include "Perspective.hlsl"
#include "ShadowPixelInput.hlsl"
#include "ShadowMap.hlsl"

ShadowPixelInput Vertex(VertexInput input)
{
    ShadowPixelInput output;
    
    float4 totalPos = float4(0.0f, 0.0f, 0.0f, 0.0f);
    bool hasBones = false;
    for (int i = 0; i < 4; i++)
    {
        uint boneId = input.boneIndices[i];
        if (boneId == -1)
        {
            break;
        }
        if (boneId >= totalBones)
        {
            totalPos = input.position;
            break;
        }
        hasBones = true;
        float4 localPos = mul(input.position, bones[boneId]);
        totalPos += localPos * input.weights[i];
    }
    
    if (!hasBones)
    {
        totalPos = input.position;
    }

   /* output.position = PerspectiveTransform(
		input.position,
		world,
		view,
		projection
	);*/
    
    totalPos = mul(totalPos, modelTransform);
    
    output.position = mul(totalPos, lightSpace);
    
    output.tex = input.tex;
    output.normal = float3(0, 0, 0);
    output.lightViewPosition = float4(0, 0, 0, 0);
    output.lightPos = float3(0, 0, 0);

    return output;
}