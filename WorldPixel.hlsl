//#include "WorldObjectCBuffer.hlsli"
#include "Pixel.hlsl"
#include "Gamma.hlsl"
//#include "ShadowMap.hlsl"
#include "PointLight.hlsli"

Texture2D albedo : register(t0);

SamplerState sampleType : register(s0);
SamplerComparisonState shadowSampler : register(s1);


TextureCube shadowMap : register(t1);
#define dx 1.0

const float3 sampleOffsetDirections[20] = {
   float3(+dx, +dx, +dx), float3(+dx, -dx, +dx), float3(-dx, -dx, +dx), float3(-dx, +dx, +dx), 
   float3(+dx, +dx, -dx), float3(+dx, -dx, -dx), float3(-dx, -dx, -dx), float3(-dx, +dx, -dx),
   float3(+dx, +dx, 0.0), float3(+dx, -dx, 0.0), float3(-dx, -dx, 0.0), float3(-dx, +dx, 0.0),
   float3(+dx, 0.0, +dx), float3(-dx, 0.0, +dx), float3(+dx, 0.0, -dx), float3(-dx, 0.0, -dx),
   float3(0.0, +dx, +dx), float3(0.0, -dx, +dx), float3(0.0, -dx, -dx), float3(0.0, +dx, -dx)
};

float Shadow(float3 pos, float4 pixelPosLightSpace, float3 normal)
{   
    float3 pixelToLight = pos - lightPos.xyz;

    //float closest = shadowMap.Sample(shadowSampler, pixelToLight).r; // get nearest in map
    
    float current = (pixelPosLightSpace.xyz / pixelPosLightSpace.w).z;
    
    float shadow = 0.0;
    int samples = 20;
    float viewDistance = length(camera.xyz - pos);
    
    float diskRadius = (1.0 + (viewDistance / 1000.0f)) / 25.0;
    //1.0 / 800.0;
    //(1.0 + (viewDistance / 1000.0f)) / 25.0;
    for (int i = 0; i < samples; i++)
    {
        shadow += shadowMap.SampleCmpLevelZero(shadowSampler, normalize(pixelToLight) + sampleOffsetDirections[i] * diskRadius, current);
    }
    return shadow / float(samples);
        /*if (current > closest)
        {
            shadow += 1.0;
        }
        else
        {
            shadow -= 1.0;
        }*/

    //return shadow / float(samples);

}

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
    
    float3 posToLight = input.fragPos - lightPos.xyz;
    
    int axis = 0; // right
    float component = 0;
    float
        xDot = abs(dot(posToLight, float3(1, 0, 0))),
        yDot = abs(dot(posToLight, float3(0, 1, 0))),
        zDot = abs(dot(posToLight, float3(0, 0, 1)));
    if (xDot > yDot)
    {
        if (xDot > zDot)
        {
            axis = 0;
            component = posToLight.x;
        }
        else
        {
            axis = 2;
            component = posToLight.z;
        }
    }
    else
    {
        if (yDot > zDot)
        {
            axis = 1;
            component = posToLight.y;
        }
        else
        {
            axis = 2;
            component = posToLight.z;
        }
    }
    axis *= 2;
    if (component < 0)
    {
        axis += 1;
    }
    
    float4 pixelPosLightSpace = mul(float4(input.fragPos, 1.0), lightSpace[axis]);
    
    float shadow = Shadow(input.fragPos, pixelPosLightSpace, normal);
    
    float pointColor = CalcPointLight(normal, input.fragPos, viewDir, texColor);
    
    float3 lighting = (ambient + (1.0 - shadow) * ((diffuse + specular) + pointColor)) * texColor;
    
    return float4(lighting, 1.0f);
}