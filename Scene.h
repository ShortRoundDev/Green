#pragma once

#include "PxPhysicsAPI.h"

#include <string>
#include <vector>

class Mesh;
class Shader;
class Camera;
class ILight;
class GameManager;
class GameObject;

using namespace physx;

class Scene
{
public:
    Scene(std::string fileName, GameManager* gameManager);
    ~Scene();

    void generateShadowMaps();
    void initEntities();
    void addRigidBodies(GameManager* gameManager);
    void draw();
    void renderMeshes();
    void update();

    Camera* getCamera();

    std::vector<GameObject*>& getGameObjects();

private:
    Shader* m_shader;
    Camera* m_camera;
    ILight* m_light;

    std::vector<Mesh*> m_brushes;
    std::vector<PxConvexMesh*> m_physicsMeshes;

    //std::vector<btRigidBody*> m_walls;
    std::vector<GameObject*> m_gameObjects;

    bool initSceneTextures();
};
