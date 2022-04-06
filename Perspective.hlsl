#ifndef __PERSPECTIVE_HLSL__
#define __PERSPECTIVE_HLSL__

float4 PerspectiveTransform(float4 position, matrix world, matrix view, matrix projection)
{
    float4 output = position;
    output = mul(output, world);
    output = mul(output, view);
    output = mul(output, projection);
    return output;
}

#endif