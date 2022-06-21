#ifndef __LIGHT_UTIL_HLSLI__
#define __LIGHT_UTIL_HLSLI__

float diffuse(
    float3 pixelPos,
    float3 normal,
    float3 lightPos,
    float roughness
)
{
    float3 dir = normalize(lightPos - pixelPos);
    float diffMag = max(dot(normal, dir), 0.0f);

    return diffMag;
}

float specular(
    float3 viewDir,
    float3 normal,
    float3 lightPos,
    float3 pixelPos,
    uint shininess
)
{
    float3 normalizedViewDir = normalize(viewDir);
    
    float3 dir = normalize(lightPos - pixelPos);
    float3 reflectDir = reflect(-dir, normal);

    float specularMag = pow(max(dot(normalizedViewDir, reflectDir), 0.0), 4);
    
    return specularMag;
}

#endif