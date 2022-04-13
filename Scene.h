#pragma once

#include "reactphysics3d/reactphysics3d.h"
#include <string>

using namespace reactphysics3d;

class Mesh;
class Shader;
class Camera;
class ILight;
class GameManager;

class Scene
{
public:
    Scene(std::string fileName, GameManager* gameManager);
    ~Scene();

    void generateShadowMaps();
    void addRigidBodies(GameManager* gameManager);
    void draw();
    void renderMeshes();
    void update();


private:
    size_t m_meshCount;
    Mesh* m_brushes;
    Shader* m_shader;
    ConvexMeshShape** m_physicsMeshes;
    size_t m_physicsMeshCount;
    Camera* m_camera;
    ILight* m_light;

    std::vector<RigidBody*> m_walls;

    bool initSceneTextures();
};
