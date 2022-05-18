//#include "WorldObjectCBuffer.hlsli"
#include "Pixel.hlsl"
#include "Gamma.hlsl"

#include "DirectionalLight.hlsli"
#include "PointLight.hlsli"

cbuffer Lights : register(b1)
{
    uint nPointLights;
    PointLight pointLights[3];
}

Texture2D albedo : register(t0);
TextureCube shadowMaps[3] : register(t1);

SamplerState sampleType : register(s0);
SamplerComparisonState shadowSampler : register(s1);



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
    uint x = 16;
    for (int i = 0; i < nPointLights; i++)
    {
        shadowAccumulator += PointLightShadow(
            shadowSampler,
            pointLights[i],
            input.pixelPos,
            camera,
            shadowMaps[i]
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
    
    float shadow = 0.0f;
    
    if (nPointLights > 0)
    {
        shadow = shadowAccumulator / float(nPointLights);
    }
    
    float3 lighting = texColor * (ambient + sunLight + (pointColor * (1.0f - shadow)));
    
    return float4(lighting, 1.0f);
}
