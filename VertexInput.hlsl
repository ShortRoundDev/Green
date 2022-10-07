#ifndef __VERTEX_INPUT_HLSLI__
#define __VERTEX_INPUT_HLSLI__

#define MAX_BONE_INFLUENCE 4

struct VertexInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
    uint4 boneIndices : BONEINDICES;
    float4 weights : WEIGHTS;
};

#endif