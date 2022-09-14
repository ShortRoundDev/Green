#include "Scene.h"

#include "GraphicsManager.h"
#include "GameManager.h"
#include "SystemManager.h"
#include "GTypes.h"
#include "Logger.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera.h"
#include "SpotLight.h"
#include "PointLight.h"
#include "MeshLightBuffer.h"
#include "MeshViewModel.h"
#include "MeshViewModelBuilder.h"
#include "Player.h"
#include "MapFile.h"
#include "ILight.h"

#include "MeshEntity.h"
#include "WorldSpawn.h"
#include "AmbientLightVolume.h"
#include "LightVolume.h"
#include "SimpleGameObject.h"
#include "Zombie.h"
#include "DebugMarkerEntity.h"

#include <algorithm>
#include "DirectionalLight.h"
#include "WavefrontMeshFactory.h"

static ::Logger logger = CreateLogger("Scene");

Scene::Scene(std::string fileName, GameManager* gameManager)
{
    initFromFiles(fileName, gameManager);
    initBrushViewModels();

    //debug: adding nav node markers
    for (auto node : m_navMesh.getRoot())
    {
        m_gameObjects.push_back(new DebugMarkerEntity(node->getSpot(), 4.0f, { 1.0f, 1.0f, 0.1f, 0.5f }));
    }
}

Scene::~Scene()
{

}

void Scene::generateShadowMaps()
{
    for (auto light : m_lights)
    {
        light->renderShadowMap(this);
    }
    m_directionalLight->renderShadowMap(this);
    //m_light2->renderShadowMap(this);
}

void Scene::initFromFiles(std::string fileName, GameManager* gameManager)
{
    // Get entities + list of mesh entities
    std::vector<MeshEntity*> meshEntities;
    initFromMapFile(fileName, meshEntities);
    // get physical world (brushes/walls/navigable surfaces)
    initFromObjFile(fileName, gameManager);

    //create graphics stuff. not sure if m_shader is needed here...
    m_camera = new Camera();
    m_shader = Graphics.getShader(L"World");

    // create worldspawn mesh entities from brushes for octree
    std::transform(m_brushes.begin(), m_brushes.end(), std::back_inserter(meshEntities), [](Mesh* mesh) {
        return new WorldSpawn(mesh);
    });

    m_tree = new Octree(meshEntities);
    logger.info("Octree Size: %ld", m_tree->size());
}

void Scene::initFromObjFile(std::string fileName, GameManager* gameManager)
{
    WavefrontMeshFactory* meshFactory = new WavefrontMeshFactory(fileName + ".obj", true, gameManager);

    std::vector<MeshActor> meshActors;
    meshFactory->createMeshes(meshActors);
    std::transform(
        meshActors.begin(), meshActors.end(),
        std::back_inserter(m_brushes),
        [](MeshActor meshActor) {
            return meshActor.mesh;
        }
    );
    delete meshFactory;
    /*
    if (!Mesh::loadObj(
        fileName + ".obj",
        m_brushes,
        &m_navMesh,
        gameManager,
        true
    ))
    {
        logger.err("Failed to load meshes!");
        return;
    }*/
}

void Scene::initBrushViewModels()
{
    for (auto brush : m_brushes)
    {
        // intermediary to build mesh viewmodel
        auto vmBuilder = MeshViewModelBuilder(brush);

        //get all lights that collide with brush
        auto box = brush->getBox();
        std::vector<MeshEntity*> result;
        m_tree->queryType(&box, result, LIGHT_VOLUME);

        // add lights to viewmodel builder
        for (auto meshEnt : result)
        {
            auto lightVolume = (LightVolume*)meshEnt;
            auto light = lightVolume->getLight();

            //check type
            if (light->getLightType() == POINT_LIGHT)
            {
                vmBuilder.addPointLight((PointLight*)light);
            } else if (light->getLightType() == SPOT_LIGHT)
            {
                vmBuilder.addSpotLight((SpotLight*)light);
            }
        }

        m_meshViewModels.push_back(vmBuilder.build());
    }
}


void Scene::initFromMapFile(std::string fileName, std::vector<MeshEntity*>& meshEntities)
{
    thread_local static char errBuff[1024];

    MF_Map map;
    sz mapfileSize;
    u8* data;
    System.readFile(fileName + ".map", &data, 0x100000, &mapfileSize); // 1mib max
    if (!MF_Parse((char*)data, &map))
    {
        MF_GetErrMessage(errBuff);
        logger.err("Failed to parse .map file! %s", errBuff);
        return;
    }
    MF_BrushDictionary dict;
    MF_GenerateMesh(&map, &dict);

    initEntities(&map, &dict, meshEntities);
    
    HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, data);
}

void Scene::initEntities(MF_Map* map, MF_BrushDictionary* dict, std::vector<MeshEntity*>& meshEntities)
{
    static thread_local char buffer[512];
    if (map == NULL)
    {
        return;
    }

    for (int i = 0; i < map->totalItems; i++)
    {
        auto item = map->items[i];
        if (!strcmp(item.classname, "info_player_start"))
        {
            auto player = Player::Create(&item);
            if (player)
            {
                m_gameObjects.push_back(player);
            }
        }
        else if (!strcmp(item.classname, "PointLight"))
        {
            auto light = PointLight::Create(&item);
            if (light)
            {
                m_lights.push_back(light);
                meshEntities.push_back(new LightVolume(light));
            }
        }
        else if (!strcmp(item.classname, "SpotLight"))
        {
            auto light = SpotLight::Create(&item);
            if (light)
            {
                m_lights.push_back(light);
                meshEntities.push_back(new LightVolume(light));
            }
        }
        else if (!strcmp(item.classname, "SunLight"))
        {
            if (m_directionalLight != nullptr)
            {
                logger.warn("Warning: sun already defined! Overwriting with 2nd sun...");
                delete m_directionalLight;
            }

            m_directionalLight = DirectionalLight::Create(&item);
        }
    }

    //m_gameObjects.push_back(new SimpleGameObject(XMFLOAT3(452.664093f, 0.600002f, -513.489319f)));
    m_gameObjects.push_back(new Zombie(XMFLOAT3(-64.0f, 32.0f, 100.0f)));

    for (int i = 0; i < dict->totalBrushes; i++)
    {
        auto item = dict->brushes + i;
        std::string name = std::string(item->name);
        if (name.starts_with("AmbientLight"))
        {
            auto light = AmbientLightVolume::Create(item);
            if (light)
            {
                meshEntities.push_back(light);
            }
        }
    }
}


void Scene::draw()
{
    generateShadowMaps();
    m_shader->use();
    m_camera->update();

    Graphics.setViewMatrix(m_camera->getView());
    Graphics.setCameraPos(m_camera->getPosition());

    if (m_directionalLight)
    {
        m_directionalLight->use(0);
    }

    renderViewModels();

    for (auto ent : m_gameObjects)
    {
        ent->draw();
    }

    /*for (auto l : m_lights)
    {
        l->draw();
    }*/
    //m_tree->draw();
}

void Scene::renderMeshes()
{
    for (u32 i = 0; i < m_brushes.size(); i++)
    {
        if (m_brushes[i]->getTexture() == NULL)
        {
            logger.warn("%d: Texture is null", i);
        }
        m_brushes[i]->draw();
    }
}

void Scene::renderEntities(Shader* shader)
{
    for (auto ent : m_gameObjects)
    {
        ent->draw(shader);
    }
}

void Scene::renderViewModels()
{
    //for (u32 i = 0; i < m_meshViewModels.size(); i++)
    //{
    //    m_meshViewModels[i]->draw();
    //}
    i32 drawn = 0;
    for (u32 i = 0; i < m_meshViewModels.size(); i++)
    {
        //if (m_brushes[i]->getTexture() == NULL)
        //{
        //    logger.warn("%d: Texture is null", i);
        //}
        auto box = m_meshViewModels[i]->getMesh()->getBox();
        if (Graphics.m_frustum.checkBox(&box))
        {
            m_meshViewModels[i]->draw();
            drawn++;
        }
    }
    

}

void Scene::update()
{
    for (auto object : m_gameObjects)
    {
        object->update();
    }
    for (auto add : m_addGameObjects)
    {
        m_gameObjects.push_back(add);
    }
    m_addGameObjects.clear();
}

Camera* Scene::getCamera()
{
    return m_camera;
}

const std::vector<GameObject*>& Scene::getGameObjects()
{
    return m_gameObjects;
}

const std::vector<ILight*>& Scene::getLights()
{
    return m_lights;
}

void Scene::putGameObject(GameObject* gameObject)
{
    m_addGameObjects.push_back(gameObject);
}

Octree* Scene::getTree()
{
    return m_tree;
}