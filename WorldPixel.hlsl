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
    float3 ambient = sun.ambient.rgb * sun.ambient.a;
    float3 sunLight = CalculateDirectionalColor(
        sun,
        texColor,
        camera.xyz,
        input.pixelPos,
        input.normal,
        0,     //todo: make this a texture or something
        0.2f    //todo: make this a texture or something
    );
    //float shadowAccumulator = 0.0f;
    float3 pointColor = float3(0.0f, 0.0f, 0.0f);
    
    //nPointLights;
    
    for (uint i = 0; i < nPointLights; i++)
    {
        float pointShadow = PointLightShadow(
            pointShadowSampler,
            pointLights[i],
            input.pixelPos,
            camera,
            pointShadowMaps[i]
        );
        
        //if (pointShadow < 1.0f)
        //{
            //shadowAccumulator += pointShadow;
            //totalLights++;
        //}
        
        //if (pointShadow < 1.0f)
        //{
            pointColor += CalcPointLight(
                0,
                0.2f,
                input.normal,
                input.pixelPos,
                camera,
                texColor,
                pointLights[i]
            ) * (1.0f - pointShadow);
        //}
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
        /*if (spotShadow >= 0 && spotShadow < 1.0f) // returns -1 if outside shadowmap
        {
            totalLights++;
            shadowAccumulator += spotShadow;
        }*/
        //if (spotShadow < 1.0f && spotShadow >= 0.0f)
        //{
            spotColor += CalcSpotLight(
                0, 0.2f,
                input.normal,
                input.pixelPos,
                camera,
                texColor,
                spotLights[j]
            ) * (1.0f - spotShadow);
        //}
    }
        
    float3 lighting = texColor * (ambient + sunLight + (pointColor + spotColor));
    
    return float4(lighting, 1.0f);
}
