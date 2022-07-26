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
        DirectionalLightBuffer light
    );

    ~AmbientLightVolume();

    const DirectionalLightBuffer& getLightDesc();

private:
    DirectionalLightBuffer m_directionalLight;
};

