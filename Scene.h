#pragma once

#include "PxPhysicsAPI.h"

#include "Octree.h"
#include "NavMesh.h"

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
class DirectionalLight;

using namespace physx;

class Scene
{
public:
    Scene(std::string fileName, GameManager* gameManager);
    ~Scene();

    void generateShadowMaps();
    void draw();
    void renderEntities(Shader* shader);
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
    NavMesh m_navMesh;

    std::vector<GameObject*> m_gameObjects;
    std::vector<GameObject*> m_addGameObjects;

    Octree* m_tree;
    DirectionalLight* m_directionalLight;

    void initFromFiles(std::string fileName, GameManager* gameManager);
    void initFromMapFile(std::string fileName, std::vector<MeshEntity*>& meshEntities);
    void initFromObjFile(std::string fileName, GameManager* gameManager);
    void initBrushViewModels();
    void initEntities(MF_Map* map, MF_BrushDictionary* dict, std::vector<MeshEntity*>& meshEntities);
};
