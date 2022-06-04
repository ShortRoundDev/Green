#pragma once

class Mesh;
class ILight;
class PointLight;
class SpotLight;
class Shader;

#include "GTypes.h"
#include "MeshLightBuffer.h"

#include <vector>

class MeshViewModel
{
public:
    MeshViewModel(
        Mesh* mesh,
        std::vector<SpotLight*> spotLights,
        std::vector<PointLight*> pointLights
    );
    ~MeshViewModel();

    void draw();

private:
    Mesh* m_mesh;
    PointLight* m_pointLights[3]; // may change later
    SpotLight* m_spotLights[3];
    u32 m_numSpotLights;
    u32 m_numPointLights;
    Shader* m_shader;

    ID3D11ShaderResourceView** m_pointLightBuffer;
    ID3D11ShaderResourceView** m_spotLightBuffer;
};

