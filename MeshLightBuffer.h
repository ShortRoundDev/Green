#pragma once

#include "PointLight.h"
#include "GTypes.h"

struct MeshLightBuffer
{
    u32 nPointLights;
    PointLightBuffer pointLights[3];
};