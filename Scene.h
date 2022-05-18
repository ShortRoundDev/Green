#pragma once

#include "PxPhysicsAPI.h"

#include "Octree.h"

#include <string>
#include <vector>

class Mesh;
class Shader;
class Camera;
class ILight;
class GameManager;
class GameObject;
class MeshViewModel;

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
    void renderViewModels();
    void update();

    Camera* getCamera();

    const std::vector<GameObject*>& getGameObjects();
    const std::vector<ILight*>& getLights();

    bool lights = true;

private:
    Shader* m_shader;
    Camera* m_camera;

    std::vector<Mesh*> m_brushes;
    std::vector<MeshViewModel*> m_meshViewModels;
    std::vector<PxConvexMesh*> m_physicsMeshes;
    std::vector<ILight*> m_lights;

    //std::vector<btRigidBody*> m_walls;
    std::vector<GameObject*> m_gameObjects;

    Octree* m_tree;

    bool initSceneTextures();
};
