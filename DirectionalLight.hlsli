#ifndef __DIRECTIONAL_LIGHT_HLSLI__
#define __DIRECTIONAL_LIGHT_HLSLI__

struct SunBuffer
{
    float4 color;
    float4 direction;
    float4 pos;
    matrix dirLightSpace;
};

struct DirectionalLight
{
    //Ambient
    float4 ambientA;
    float4 ambientB;
    float4 ambientDirection;
    
    // Sun
    SunBuffer sun;
    
    //Also ambient but I dont wanna pack this shit
    float hardness;
};

float DirectionalLightShadow(
    SamplerState shadowSampler,
    matrix lightSpace,
    float3 pos,
    float4 camera,
    Texture2D shadowMap
)
{
    float4 pixelPosLightSpace = mul(float4(pos, 1.0f), lightSpace);
    float3 projCoords = pixelPosLightSpace.xyz / pixelPosLightSpace.w;
    
    float current = projCoords.z;
    projCoords = (projCoords * 0.5f + 0.5f);
    projCoords.y = projCoords.y * -1.0f + 1.0f;
    float shadowAcc = 0.0f;
    
    float2 texel;
    shadowMap.GetDimensions(texel.x, texel.y);
    texel = 1.0f / texel;
    

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


float3 DiffuseDirectionalLight(DirectionalLight light, float3 normal, float roughness)
{
    float dp = dot(normalize(light.sun.direction.xyz), normal);
    if (dp >= 0.0f)
    {
        return 0.0f;
    }
    float diffMagnitude = max(abs(dp), roughness);
    return (light.sun.color.rgb * light.sun.color.a) * diffMagnitude;
}

float3 SpecularDirectionalLight(
    DirectionalLight light,
    float3 camera,
    float3 position,
    float3 normal,
    uint shininess
)
{
    float dp = dot(normalize(light.sun.direction.xyz), normal);
    if (dp >= 0.0f)
    {
        return 0.0f;
    }

    float3 viewDir = normalize(camera - position);
    float3 reflectDir = reflect(-normalize(light.sun.direction.xyz), normal);
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    return (light.sun.color.rgb * light.sun.color.a) * (0.5f * specularStrength);
}

float3 CalculateDirectionalColor(
    DirectionalLight light,
    float3 camera,
    float3 position,
    float3 normal,
    uint shininess,
    float roughness
)
{
    return
        DiffuseDirectionalLight(light, normal, roughness) +
        SpecularDirectionalLight(light, camera, position, normal, shininess);
}

#endif