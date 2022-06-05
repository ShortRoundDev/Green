//#include "WorldObjectCBuffer.hlsli"
#include "Pixel.hlsl"
#include "Gamma.hlsl"

#include "DirectionalLight.hlsli"
#include "PointLight.hlsli"
#include "SpotLight.hlsli"

cbuffer Lights : register(b1)
{
    uint nPointLights;
    PointLight pointLights[3];
    float3 padding;
    uint nSpotLights;
    SpotLight spotLights[3];
}

Texture2D albedo : register(t0);

TextureCube pointShadowMaps[3] : register(t1);
Texture2D spotShadowMaps[3] : register(t4);

SamplerState sampleType : register(s0);
SamplerComparisonState pointShadowSampler : register(s1);
SamplerState spotShadowSampler : register(s2);

float4 Pixel(PixelInput input) : SV_TARGET
{
    float3 texColor = albedo.Sample(sampleType, input.tex).rgb;
    float3 ambient = sun.ambient.rgb;
    float3 sunLight = CalculateDirectionalColor(
        sun,
        texColor,
        camera.xyz,
        input.pixelPos,
        input.normal,
        16,     //todo: make this a texture or something
        0.2f    //todo: make this a texture or something
    );
    float shadowAccumulator = 0.0f;
    float3 pointColor = float3(0.0f, 0.0f, 0.0f);
    
    uint totalLights = nPointLights;
    
    for (uint i = 0; i < nPointLights; i++)
    {
        shadowAccumulator += PointLightShadow(
            pointShadowSampler,
            pointLights[i],
            input.pixelPos,
            camera,
            pointShadowMaps[i]
        );
        
        pointColor += CalcPointLight(
            16,
            0.2f,
            input.normal,
            input.pixelPos,
            camera,
            texColor,
            pointLights[i]
        );
    }
    
    float3 spotColor = float3(0.0f, 0.0f, 0.0f);
    for (uint j = 0; j < nSpotLights; j++)
    {
        float spotShadow = SpotLightShadow(
            spotShadowSampler,
            spotLights[j],
            input.pixelPos,
            camera,
            spotShadowMaps[j]
        );
        if (spotShadow >= 0) // returns -1 if outside shadowmap
        {
            totalLights++;
            shadowAccumulator += spotShadow;
        }
        
        spotColor += CalcSpotLight(
            16, 0.2f,
            input.normal,
            input.pixelPos,
            camera,
            texColor,
            spotLights[j]
        );
    }
    
    float shadow = 0.0f;
        
    if (totalLights > 0)
    {
        shadow = min(1.0f, max(shadowAccumulator / totalLights, 0.0f));
    }
    
    float3 lighting = texColor * (ambient + sunLight + ((pointColor + spotColor) * (1.0f - shadow)));
    
    return float4(lighting, 1.0f);
}
