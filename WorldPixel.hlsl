//#include "WorldObjectCBuffer.hlsli"
#include "Pixel.hlsl"
#include "Gamma.hlsl"

#include "Light.hlsli"

#include "DirectionalLight.hlsli"
#include "PointLight.hlsli"
#include "SpotLight.hlsli"

cbuffer Lights : register(b2)
{
    uint nPointLights;
    PointLight pointLights[3];
    float3 padding;
    uint nSpotLights;
    SpotLight spotLights[3];
}

Texture2D   albedo                      : register(t0);
TextureCube pointShadowMaps[MAX_LIGHTS] : register(t1);
Texture2D   spotShadowMaps[MAX_LIGHTS]  : register(SLOT_OFF(t, SPOTLIGHT_OFFSET));
Texture2D   sunShadowMap                : register(SLOT_OFF(t, SUNLIGHT_OFFSET));


SamplerState sampleType                     : register(s0);
SamplerComparisonState pointShadowSampler   : register(s1);
SamplerState spotShadowSampler              : register(s2);

float4 Pixel(PixelInput input) : SV_TARGET
{
    float3 texColor = albedo.Sample(sampleType, input.tex).rgb;
    float3 ambient = (
            dot(
                normalize(dirLight.ambientDirection.xyz),
                input.normal
            ) * 0.5f + 0.5f - dirLight.hardness
        ) /
        (1.0f - dirLight.hardness) *
        (dirLight.ambientA.rgb * dirLight.ambientA.a) +
        (dirLight.ambientB.rgb * dirLight.ambientB.a);
    
    float3 sunLight = float3(0.0f, 0.0f, 0.0f);
    if (dirLight.sun.color.a != 0.0f)
    {
        SpotLight tmp =
        {
            dirLight.sun.dirLightSpace,
            dirLight.sun.color,
            dirLight.sun.pos,
            dirLight.sun.direction,
            0.0f, 0.0f, 0.0f, 0.0f
        };
    
        float dirShadow = SpotLightShadow(
            pointShadowSampler,
            tmp,
            input.pixelPos,
            camera,
            sunShadowMap
        );
        
        sunLight = CalculateDirectionalColor(
            dirLight,
            camera.xyz,
            input.pixelPos,
            input.normal,
            0, //todo: make this a texture or something
            0.2f //todo: make this a texture or something
        ) * dirShadow;
    }

    float3 pointColor = float3(0.0f, 0.0f, 0.0f);
    
    for (uint i = 0; i < nPointLights; i++)
    {
        float pointShadow = PointLightShadow(
            pointShadowSampler,
            pointLights[i],
            input.pixelPos,
            camera,
            pointShadowMaps[i]
        );
                
        pointColor += CalcPointLight(
            0,
            0.2f,
            input.normal,
            input.pixelPos,
            camera,
            pointLights[i]
        ) * (1.0f - pointShadow);
    }
    
    float3 spotColor = float3(0.0f, 0.0f, 0.0f);
    for (uint j = 0; j < nSpotLights; j++)
    {
        float spotShadow = SpotLightShadow(
            pointShadowSampler,
            spotLights[j],
            input.pixelPos,
            camera,
            spotShadowMaps[j]
        );
                
        spotColor += CalcSpotLight(
            0, 0.2f,
            input.normal,
            input.pixelPos,
            camera,
            spotLights[j]
        ) * spotShadow;
    }
    
    float3 lighting = texColor * (ambient + sunLight + pointColor + spotColor);
    
    return float4(lighting, 1.0f);
}
