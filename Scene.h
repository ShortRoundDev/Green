#pragma once

#include "PxPhysicsAPI.h"

#include "Octree.h"

#include <string>
#include <vector>

struct MF_Map;
struct MF_BrushDictionary;
class Mesh;
class MeshEntity;
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
    void draw();
    void renderMeshes();
    void renderViewModels();
    void update();

    Camera* getCamera();
    Octree* getTree();

    const std::vector<GameObject*>& getGameObjects();
    const std::vector<ILight*>& getLights();
    void putGameObject(GameObject* gameObject);

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
    std::vector<GameObject*> m_addGameObjects;

    Octree* m_tree;

    void initFromMapFile(std::string fileName, std::vector<MeshEntity*>& meshEntities);
    void initEntities(MF_Map* map, MF_BrushDictionary* dict, std::vector<MeshEntity*>& meshEntities);
};
