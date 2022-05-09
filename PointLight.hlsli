#ifndef __POINT_LIGHT_HLSLI__
#define __POINT_LIGHT_HLSLI__

cbuffer PointLight : register(b1)
{
    matrix lightSpace[6];
    float4 lightColor;
    float4 lightPos;
};

float3 CalcPointLight(float3 normal, float3 pixelPos, float3 viewDir, float3 texel)
{
    float3 dir = normalize(lightPos.xyz - pixelPos);
    float diff = max(dot(normal, dir), 0.0);
    
    float3 reflectDir = reflect(-dir, normal);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), 16);

    float distance = length(lightPos.xyz - pixelPos);
    float attenuation = 1.0f / (0.001f + 0.00009f * distance + 0.000032f * (distance * distance));
        
    float3 diffuseLight = (0.2f * diff * texel) * attenuation;
    float3 specularLight = (0.5f * specular * texel) * attenuation;
    
    return (diffuseLight + specularLight);
}
#endif