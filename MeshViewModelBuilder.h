#pragma once

class MeshViewModel;
class Mesh;
class ILight;
class SpotLight;
class PointLight;

#include <vector>

class MeshViewModelBuilder
{
public:
    MeshViewModelBuilder(Mesh* mesh);

    const std::vector<SpotLight*>& getSpotLights();
    const std::vector<PointLight*>& getPointLights();
    void addSpotLight(SpotLight* spotLight);
    void addPointLight(PointLight* pointLight);
    
    MeshViewModel* build();
    
private:
    Mesh* m_mesh;
    std::vector<SpotLight*> m_spotLights;
    std::vector<PointLight*> m_pointLights;

};