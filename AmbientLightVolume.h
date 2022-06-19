#pragma once
#include "MeshEntity.h"

#include "GlobalBuffer.h"

struct MF_MeshEntity;

class AmbientLightVolume : public MeshEntity
{
public:

    static AmbientLightVolume* Create(MF_MeshEntity* entity);

    AmbientLightVolume(
        Mesh* mesh,
        DirectionalLight light
    );

    ~AmbientLightVolume();

    const DirectionalLight& getLightDesc();

private:
    DirectionalLight m_directionalLight;
};

