#pragma once
#include "MeshEntity.h"
#include "ILight.h"

class LightVolume : public MeshEntity
{
public:
    LightVolume(ILight* light);
    virtual ~LightVolume();

    ILight* getLight();

private:
    ILight* m_light;
};

