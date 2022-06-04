#pragma once

#include "PointLight.h"
#include "SpotLight.h"
#include "GTypes.h"

struct MeshLightBuffer
{
    u32 nPointLights;
    PointLightBuffer pointLights[3];
    XMFLOAT3 padding;
    u32 nSpotLights;
    SpotLightBuffer spotLights[3];
};