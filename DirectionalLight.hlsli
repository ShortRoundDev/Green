#ifndef __DIRECTIONAL_LIGHT_HLSLI__
#define __DIRECTIONAL_LIGHT_HLSLI__

struct DirectionalLight
{
    float4 ambientA;
    float4 ambientB;
    float4 ambientDirection;
    float4 color;
    float4 direction;
    float hardness;
};

float3 DiffuseDirectionalLight(DirectionalLight light, float3 normal, float roughness)
{
    float diffMagnitude = max(dot(normalize(light.direction.xyz), normal), roughness);
    return (light.color.rgb * light.color.a) * diffMagnitude;
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
    float3 reflectDir = reflect(-normalize(light.direction.xyz), normal);
    float specularStrength = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    return (light.color.rgb * light.color.a) * (0.5f * specularStrength);
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