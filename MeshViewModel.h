#pragma once

class Mesh;
class ILight;
class Shader;

#include "GTypes.h"
#include "MeshLightBuffer.h"

#include <vector>

class MeshViewModel
{
public:
    MeshViewModel(Mesh* mesh, std::vector<ILight*> lights);
    ~MeshViewModel();

    void draw();

private:
    Mesh* m_mesh;
    ILight* m_lights[3]; // may change later
    u32 m_numLights;
    Shader* m_shader;
};

