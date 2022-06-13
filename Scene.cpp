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
#include "Player.h"

#include "MapFile.h"

static ::Logger logger = CreateLogger("Scene");

Scene::Scene(std::string fileName, GameManager* gameManager)
{
    initSceneTextures();

    MF_Map map;
    size_t mapfileSize;
    u8* data;
    System.readFile(fileName + ".map", &data, 0x100000, &mapfileSize); // 1mib max
    
    if (!MF_Parse((char*)data, &map))
    {
        logger.err("Failed to parse .map file!");
        return;
    }
    MF_BrushDictionary dict;

    MF_GenerateMesh(&map, &dict);
    
    initEntities(&map);

    //destroy map
    //destroy text
    HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, data);

    if (!Mesh::createFromFile(
        fileName + ".obj",
        m_brushes,
        gameManager,
        true
    ))
    {
        logger.err("Failed to load meshes!");
        return;
    }
    /*Vector3 _pos = Vector3(0, 0, 0);
    Quaternion o = Quaternion::identity();
    Transform t(_pos, o);
    auto body = gameManager->getPhysicsWorld()->createRigidBody(t);
    body->setType(BodyType::STATIC);

    auto box = gameManager->getPhysicsCommon()->createBoxShape(Vector3(128, 8, 128));
    auto collider = body->addCollider(box, Transform::identity());*/

    m_camera = new Camera();
    m_shader = Graphics.getShader(L"World");

    //m_light = new SpotLight({ 64, 128.0f, -64, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 800, 800, { -1.0f, 0, 1.0f, 1.0f });
    //m_lights.push_back(new PointLight({ 64, 128, -64, 1 }, { 0.196f * 3.0f, 0.223f * 3.0f, 0.286f * 3.0f, 1 }, 1600, 1600, 512, 0.1));
    //m_lights.push_back(new PointLight({ 100, 100, -40, 1 }, { 1.0f, 1.0f, 0, 1 }, 128, 128, 128, 0.1));
    //m_lights.push_back(new PointLight({ 0, 100, -40, 1 }, { 1.0f, 1.0f, 0, 1 }, 512, 512, 128, 0.1));
    //m_lights.push_back(new PointLight({ 100, 80, -440, 1 }, { 0.3f, 1.0f, 0, 1 }, 128, 128, 128, 0.1));
    //m_lights.push_back(new SpotLight({ 32, 100, -80, 1 }, { 0.0f, 0.0f, 1.0f, 1 }, 128, 128, { 3.0f, -1.0f, 0.0f, 0 }, 512, 64, 0.1f));
    //m_lights.push_back(new SpotLight({ 32, 100, -20, 1 }, { 1.0f, 0.0f, 0.0f, 1 }, 128, 128, { 3.0f, -1.0f, 0.0f, 0 }, 512, 64, 0.1f));
    //m_lights.push_back(new SpotLight({ 0, 0, 0, 1 }, { 0.196f * 3.0f, 0.223f * 3.0f, 0.286f * 3.0f, 1 }, 1600, 1600, { 0, 1, 0, 1 }, 10, 10, 1.0f));
    
    //m_light2 = new PointLight({ 128, 200, 0, 1 }, { 0.196f * 3.0f, 0.223f * 3.0f, 0.286f * 3.0f, 1 }, 1600, 1600);

    m_tree = new Octree(m_brushes);
    for (auto light : m_lights)
    {
        auto box = light->getBounds();
        std::vector<Mesh*> result;

        m_tree->query(&box, result);

        for (auto mesh : result)
        {
            mesh->addLight(light);
        }
    }

    for (auto mesh : m_brushes)
    {
        m_meshViewModels.push_back(mesh->getViewModel());
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
    //m_light2->renderShadowMap(this);
}

void Scene::initEntities(MF_Map* map)
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
            }
        }
        else if (!strcmp(item.classname, "SpotLight"))
        {
            
            auto light = SpotLight::Create(&item);
            if (light)
            {
                m_lights.push_back(light);
            }
        }
    }
}


void Scene::draw()
{
    m_shader->use();
    m_camera->update();

    Graphics.setViewMatrix(m_camera->getView());
    Graphics.setCameraPos(m_camera->getPosition());

    renderViewModels();
    for (auto l : m_lights)
    {
        l->draw();
    }
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

void Scene::renderViewModels()
{
    //for (u32 i = 0; i < m_meshViewModels.size(); i++)
    //{
    //    m_meshViewModels[i]->draw();
    //}
    for (u32 i = 0; i < m_meshViewModels.size(); i++)
    {
        //if (m_brushes[i]->getTexture() == NULL)
        //{
        //    logger.warn("%d: Texture is null", i);
        //}
        m_meshViewModels[i]->draw();
    }

}

void Scene::update()
{
    for (auto object : m_gameObjects)
    {
        object->update();
    }
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

////////// PRIVATE //////////

bool Scene::initSceneTextures()
{
    Graphics.putTexture("textures\\DevFloor1.png", new Texture("textures/DevFloor1.png"));
    Graphics.putTexture("textures\\DevFloor2.png", new Texture("textures/DevFloor2.png"));
    Graphics.putTexture("textures\\DevWall1.png", new Texture("textures/DevWall1.png"));
    Graphics.putTexture("textures\\DevWall2.png", new Texture("textures/DevWall2.png"));
    Graphics.putTexture("textures\\sprites\\001-lightbulb.png", new Texture("textures/sprites/001-lightbulb.png"));
    Graphics.putTexture("textures\\sprites\\002-spotlight.png", new Texture("textures/sprites/002-spotlight.png"));

    Graphics.putTexture("textures\\carpet\\carpetfloorblack.png", new Texture("textures/carpet/carpetfloorblack.png"));
    Graphics.putTexture("textures\\carpet\\carpetfloorred.png", new Texture("textures/carpet/carpetfloorred.png"));
    Graphics.putTexture("textures\\carpet\\carpetfloorwhite.png", new Texture("textures/carpet/carpetfloorwhite.png"));
    Graphics.putTexture("textures\\paint\\plasterwallbrown2.png", new Texture("textures/paint/plasterwallbrown2.png"));
    Graphics.putTexture("textures\\paint\\plasterwallpink.png", new Texture("textures/paint/plasterwallpink.png"));
    Graphics.putTexture("textures\\paint\\plasterwallwhite.png", new Texture("textures/paint/plasterwallwhite.png"));
    Graphics.putTexture("textures\\tile\\tilefloor1.png", new Texture("textures/tile/tilefloor1.png"));

    return true;
}