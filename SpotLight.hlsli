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
    SamplerComparisonState shadowSampler,
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

    float shadow = 0.0f;
    float2 resolution;
    shadowMap.GetDimensions(resolution.x, resolution.y);
    
   // float2 grad = frac(projCoords.xy * resolution.x + 0.5f);
    
//    const int FILTER_SIZE = 1;
    
  //  for (int i = -FILTER_SIZE; i <= FILTER_SIZE; i++)
    //{
      //  for (int j = -FILTER_SIZE; j < FILTER_SIZE; j++)
       // {
         //   float4 tmp = shadowMap.Gather(shadowSampler, projCoords.xy + float2(i, j) * ((float2(1.0f / (resolution.x), 1.0f / (resolution.x)))));
         //   tmp.x = tmp.x < current ? 0.0f : 1.0f;
          //  tmp.y = tmp.y < current ? 0.0f : 1.0f;
           // tmp.z = tmp.z < current ? 0.0f : 1.0f;
            //tmp.w = tmp.w < current ? 0.0f : 1.0f;
            
     //       shadow += lerp(lerp(tmp.w, tmp.z, grad.x), lerp(tmp.x, tmp.y, grad.x), grad.y);
        //}
   // }
  
    //shadow = shadowMap.SampleCmp(shadowSampler, projCoords.xy, current).r;
    
    
    
    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= 1; j++)
        {
            shadow += shadowMap.SampleCmpLevelZero(shadowSampler, projCoords.xy + float2(i, j) * float2(1.0f / resolution.x, 1.0f / resolution.y), current).r;
        }
    }
    return shadow / 9;
    //return 1.0f - (shadow / (float) ((2 * FILTER_SIZE) * (2 * FILTER_SIZE + 1)));
}

float3 CalcSpotLight(
    uint shininess,
    float roughness,
    float3 normal,
    float3 pixelPos,
    float4 camera,
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
    float specularMagnitude = specular(
        camera.xyz - pixelPos,
        normal,
        light.pos.xyz,
        pixelPos,
        shininess
    );

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
    float3 diffuseLight = ((0.5f * diffMagnitude * (1.0f - attenuation)) * light.color.rgb) * light.color.a;
    //specular color
    float3 specularLight = ((0.2f * specularMagnitude * (1.0f - attenuation)) * light.color.rgb) * light.color.a;
    
    return (diffuseLight + specularLight);
}

#endif