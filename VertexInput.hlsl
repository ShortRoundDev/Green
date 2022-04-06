#ifndef __VERTEX_INPUT_HLSLI__
#define __VERTEX_INPUT_HLSLI__

struct VertexInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 tex : TEXCOORD0;
};

#endif