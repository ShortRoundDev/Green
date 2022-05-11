#ifndef __POINT_LIGHT_HLSLI__
#define __POINT_LIGHT_HLSLI__

static const float3 sampleOffsetDirections[20] =
{
   float3(+1.0f, +1.0f, +1.0f), float3(+1.0f, -1.0f, +1.0f), float3(-1.0f, -1.0f, +1.0f), float3(-1.0f, +1.0f, +1.0f),
   float3(+1.0f, +1.0f, -1.0f), float3(+1.0f, -1.0f, -1.0f), float3(-1.0f, -1.0f, -1.0f), float3(-1.0f, +1.0f, -1.0f),
   float3(+1.0f, +1.0f, +0.0f), float3(+1.0f, -1.0f, +0.0f), float3(-1.0f, -1.0f, +0.0f), float3(-1.0f, +1.0f, +0.0f),
   float3(+1.0f, +0.0f, +1.0f), float3(-1.0f, +0.0f, +1.0f), float3(+1.0f, +0.0f, -1.0f), float3(-1.0f, +0.0f, -1.0f),
   float3(+0.0f, +1.0f, +1.0f), float3(+0.0f, -1.0f, +1.0f), float3(+0.0f, -1.0f, -1.0f), float3(+0.0f, +1.0f, -1.0f)
};

struct PointLight
{
    matrix view[6];
    float4 color;
    float4 position;
    float constantDropoff;
    float linearDropoff;
    float quadraticDropoff;
};

float3 CalcPointLight(
    uint shininess,
    float roughness,
    float3 normal,
    float3 pixelPos,
    float4 camera,
    float3 textureColor,
    PointLight light
)
{
    float3 viewDirection = normalize(camera.xyz - pixelPos);
    
    //diffuse magnitude
    float3 dir = normalize(light.position.xyz - pixelPos);
    float diffMagnitude = max(dot(normal, dir), roughness);
    
    //specular magnitude
    float3 reflectDir = reflect(-dir, normal);
    float specularMagnitude = pow(max(dot(viewDirection, reflectDir), 0.0), shininess);

    //dropoff
    float distance = length(light.position.xyz - pixelPos);
    float attenuation = 1.0f / (
        light.constantDropoff +
        (light.linearDropoff * distance) +
        (light.quadraticDropoff * (distance * distance))
    );

    //diffuse color
    float3 diffuseLight = textureColor * (0.2f * diffMagnitude) * attenuation * light.color.rgb;
    //specular color
    float3 specularLight = textureColor * (0.5f * specularMagnitude * attenuation) * light.color.rgb;
    
    return (diffuseLight + specularLight);
}

float4 MapPointToCubeFace(float3 position, PointLight light)
{
    float3 positionToLight = position - light.position.xyz;
    
    int axis = 0; // right
    float component = 0;
    float
        xDot = abs(dot(positionToLight, float3(1, 0, 0))),
        yDot = abs(dot(positionToLight, float3(0, 1, 0))),
        zDot = abs(dot(positionToLight, float3(0, 0, 1)));
    if (xDot > yDot)
    {
        if (xDot > zDot)
        {
            axis = 0;
            component = positionToLight.x;
        }
        else
        {
            axis = 2;
            component = positionToLight.z;
        }
    }
    else
    {
        if (yDot > zDot)
        {
            axis = 1;
            component = positionToLight.y;
        }
        else
        {
            axis = 2;
            component = positionToLight.z;
        }
    }
    axis *= 2;
    if (component < 0)
    {
        axis += 1;
    }
    
    return mul(float4(position, 1.0f), light.view[axis]);

}

float PointLightShadow(
    SamplerComparisonState shadowSampler,
    PointLight light,
    float3 pos,
    float4 camera,
    TextureCube shadowMap
)
{
    float4 pixelPosLightSpace = MapPointToCubeFace(pos, light);
    
    float current = (pixelPosLightSpace.xyz / pixelPosLightSpace.w).z;
    float shadowAccumulator = 0.0f;
    
    int samples = 20; //make this configurable?
    float diskRadius = 0.001f;
    
    for (int i = 0; i < samples; i++)
    {
        shadowAccumulator += shadowMap.SampleCmpLevelZero(
            shadowSampler,
            normalize(pos.xyz - light.position.xyz) +
                (sampleOffsetDirections[i] * diskRadius),
            current
        ).r;
    }
    return shadowAccumulator / float(samples);
}

#endif