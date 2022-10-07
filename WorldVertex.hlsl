#include "Vertex.hlsl"

#include "Perspective.hlsl"
//#include "ShadowMap.hlsl"
#include "PointLight.hlsli"

PixelInput Vertex(VertexInput input)
{
    PixelInput output;
    //output.position = float4(input.position.xyz, 1.0);
    float4 totalPos = input.position;
    float weights[4] = { input.weights.x, input.weights.y, input.weights.z, input.weights.w };
    int boneIds[4] = { input.boneIndices.x, input.boneIndices.y, input.boneIndices.z, input.boneIndices.w };
    bool hasBones = false;

    float3 posL = float3(0.0f, 0.0f, 0.0f);
    float3 normalL = float3(0.0f, 0.0f, 0.0f);
    float3 tangentL = float3(0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 4; i++)
    {
        int boneId = boneIds[i];
        if (boneId == -1)
        {
            break;
        }
        hasBones = true;
        posL += weights[i] * mul(float4(input.position.xyz, 1.0f), bones[boneId]).xyz;
        normalL += weights[i] * mul(float4(input.normal.xyz, 1.0f), bones[boneId]).xyz;
        /*
        uint boneId = boneIds[i];
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
        totalPos += localPos * weights[i];*/
    }
    
    if (hasBones)
    {
        totalPos = float4(posL, 1.0f);
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