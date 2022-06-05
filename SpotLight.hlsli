#ifndef __SPOT_LIGHT_HLSLI__
#define __SPOT_LIGHT_HLSLI

#include "LightUtil.hlsli"

struct SpotLight
{
    matrix lightSpace;
    float4 color;
    float4 pos;
    float4 direction;
    float length;
    float radius;
    float cutoff;
    float cosFullAngle;
};

float SpotLightShadow(
    SamplerState shadowSampler,
    SpotLight light,
    float3 pos,
    float4 camera,
    Texture2D shadowMap
)
{
    float4 pixelPosLightSpace = mul(float4(pos, 1.0f), light.lightSpace);
    float3 projCoords = pixelPosLightSpace.xyz / pixelPosLightSpace.w;
    
    float current = projCoords.z;
    projCoords = (projCoords * 0.5f + 0.5f);
    projCoords.y = projCoords.y * -1.0f + 1.0f;
    float shadowAcc = 0.0f;
    
    float2 texel;
    shadowMap.GetDimensions(texel.x, texel.y);
    texel = 1.0f / texel;
    
    if (projCoords.x <= texel.x + 0.0005f || projCoords.x >= 1.0f - (texel.x + 0.0005f) || projCoords.y <= (texel.y + 0.0005f) || projCoords.y >= 1.0f - (texel.y + 0.0005f))
    {
        return -1;
    }
    

    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float pcfDepth = shadowMap.Sample(shadowSampler, projCoords.xy + float2(x, y) * texel).r;
            shadowAcc += current > pcfDepth ? 1.0f : 0.0f;
        }
    }
    return shadowAcc / 9.0f;
}

float3 CalcSpotLight(
    uint shininess,
    float roughness,
    float3 normal,
    float3 pixelPos,
    float4 camera,
    float3 textureColor,
    SpotLight light
)
{
    float3 lightDir = normalize(light.direction.xyz);
    float3 pixelPosLightOrigin = pixelPos - light.pos.xyz;
    float dp = dot(pixelPosLightOrigin, lightDir) / length(pixelPosLightOrigin);
    if (dp <= light.cosFullAngle)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    
    float theta = dot(lightDir, normalize(-lightDir));
    float epsilon = light.cutoff - 0.91f;
    float intensity = clamp((theta - 0.91f) / epsilon, 0.0f, 1.0f);
    
    float diffMagnitude = diffuse(pixelPos, normal, light.pos.xyz, roughness);
    float specularMagnitude = specular(camera.xyz - pixelPos, normal, light.pos.xyz, pixelPos, shininess);

    //dropoff
    float distance = dot(pixelPos, lightDir);
    //float distance = length(diff);
    float d = max(distance - light.radius, 0);
    
    float range = 1.0f - light.cosFullAngle;
    float dist = 1.0f - ((1.0f - dp) / range);
    
    float denom = dist + 1;
    float attenuation = 1 / (denom * denom);
    attenuation = (attenuation - light.cutoff) / (1 - light.cutoff);
    attenuation = max(attenuation, 0);
    
    //diffuse color
    float3 diffuseLight = textureColor * (0.2f * diffMagnitude * (1.0f - attenuation)) * light.color.rgb;
    //specular color
    float3 specularLight = textureColor * (0.5f * specularMagnitude * (1.0f - attenuation)) * light.color.rgb;
    
    return (diffuseLight) + (specularLight);
}

#endif