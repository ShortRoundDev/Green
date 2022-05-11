#ifndef __DIRECTIONAL_LIGHT_HLSLI__
#define __DIRECTIONAL_LIGHT_HLSLI__

struct DirectionalLight
{
    float4 ambient;
    float4 color;
    float4 direction;
};

float3 DiffuseDirectionalLight(DirectionalLight light, float3 normal, float roughness)
{
    float diffMagnitude = max(dot(light.direction.xyz, normal), roughness); // 0.2f configurable?
    return light.color * diffMagnitude;
}

float3 SpecularDirectionalLight(
    DirectionalLight light,
    float3 camera,
    float3 position,
    float3 normal,
    uint shininess
)
{
    float3 viewDir = normalize(camera - position);
    float3 reflectDir = reflect(-light.direction.xyz, normal);
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    return light.color.rgb * (0.5f * specularStrength);
}

float3 CalculateDirectionalColor(
    DirectionalLight light,
    float3 textureColor,
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