#include "Vertex.hlsl"

#include "Perspective.hlsl"
//#include "ShadowMap.hlsl"
#include "PointLight.hlsli"

PixelInput Vertex(VertexInput input)
{
    PixelInput output;
    //output.position = float4(input.position.xyz, 1.0);
    float4 totalPos = float4(0.0f, 0.0f, 0.0f, 0.0f);
    bool hasBones = false;
    /*for (int i = 0; i < 0; i++)
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
    }*/
    
    if (!hasBones)
    {
        totalPos = input.position;
    }
    
    output.normal = input.normal;
    output.tex = input.tex;
    
    totalPos = mul(totalPos, modelTransform);
    output.pixelPos = totalPos.xyz;
    
    output.position = PerspectiveTransform(
		totalPos,
		world,
		view,
		projection
	);

    return output;
}