#ifndef __HITBOX_BUFFER_HLSLI__
#define __HITBOX_BUFFER_HLSLI__

cbuffer HitboxBuffer : register(b2)
{
    float3 pos;
    float radius;
    float4 color;
};

#endif