//#include "WorldObjectCBuffer.hlsli"
#include "Pixel.hlsl"
#include "Gamma.hlsl"
#include "ShadowMap.hlsl"

Texture2D albedo : register(t0);

SamplerState sampleType;

/*float Shadow(float3 pos, float3 lightDir, float3 normal)
{
    float3 pixelToLight = pos - lightPos.xyz;
    float closest = shadowMap.Sample(sampleType, pixelToLight).r;
    return closest;
}*/

float4 Pixel(PixelInput input) : SV_TARGET
{
    float3 texColor = albedo.Sample(sampleType, input.tex).rgb;
    float3 normal = normalize(input.normal);
    float3 ambient = 0.05f * ambientLightColor;
    
    // diffuse
    //float3 lightDir = normalize(lightPos.xyz - input.fragPos);
    float diff = max(dot(lightDirection.xyz, normal), 0.2f);
    float3 diffuse = diff * ambientLightColor;

    //Specular
    float specularStrength = 0.5;
    float3 viewDir = normalize(camera.xyz - input.fragPos);
    float3 reflectDir = reflect(-lightDirection.xyz, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    float specular = specularStrength * spec * ambientLightColor;
    
    //float shadow = Shadow(input.fragPos, lightDir, normal);
    float3 lighting = (ambient + diffuse + specular) * texColor;
    
    return float4(lighting, 1.0f);
}