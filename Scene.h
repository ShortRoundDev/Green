#pragma once

#include <string>

class Mesh;
class Shader;
class Camera;

class Scene
{
public:
    Scene(std::string fileName);
    ~Scene();

    void draw();
    void renderMeshes();
    void update();

private:
    size_t m_meshCount;
    Mesh* m_brushes;
    Shader* m_shader;
    Camera* m_camera;

    bool initSceneTextures();
};
