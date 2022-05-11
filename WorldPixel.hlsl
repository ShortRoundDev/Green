//#include "WorldObjectCBuffer.hlsli"
#include "Pixel.hlsl"
#include "Gamma.hlsl"

#include "DirectionalLight.hlsli"
#include "PointLight.hlsli"

Texture2D albedo : register(t0);

TextureCube shadowMap : register(t1);
TextureCube shadowMap2 : register(t2);
TextureCube shadowMap3 : register(t3);

SamplerState sampleType : register(s0);
SamplerComparisonState shadowSampler : register(s1);

cbuffer Lights : register(b1)
{
    uint nPointLights;
    PointLight pointLights[3];
}

float4 Pixel(PixelInput input) : SV_TARGET
{
    float3 texColor = albedo.Sample(sampleType, input.tex).rgb;
    float3 ambient = sun.ambient;
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
    TextureCube t = shadowMap;
    uint x = 16;
    for (int i = 0; i < nPointLights; i++)
    {
        if (i == 0)
        {
            shadowAccumulator += PointLightShadow(
                shadowSampler,
                pointLights[i],
                input.pixelPos,
                camera,
                shadowMap
            );
        }
        else
        {
            shadowAccumulator += PointLightShadow(
                shadowSampler,
                pointLights[i],
                input.pixelPos,
                camera,
                shadowMap2
            );
        }
        
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
    
    float shadow = 0.0f;
    
    if (nPointLights > 0)
    {
        shadow = shadowAccumulator / float(nPointLights);
    }
    
    float3 lighting = texColor * (ambient + sunLight + (pointColor * (1.0f - shadow)));
    
    return float4(lighting, 1.0f);
}
