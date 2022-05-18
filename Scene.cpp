#include "Scene.h"

#include "GraphicsManager.h"
#include "GameManager.h"
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

static ::Logger logger = CreateLogger("Scene");

Scene::Scene(std::string fileName, GameManager* gameManager)
{
    initSceneTextures();
    if (!Mesh::createFromFile(
        fileName,
        m_brushes,
        m_physicsMeshes,
        gameManager
    ))
    {
        logger.err("Failed to load meshes!");
        return;
    }
    addRigidBodies(gameManager);

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
    m_lights.push_back(new PointLight({ 64, 128, -64, 1 }, { 0.196f * 3.0f, 0.223f * 3.0f, 0.286f * 3.0f, 1 }, 1600, 1600, 512, 0.1));
    m_lights.push_back(new PointLight({ 128, 200, 0, 1 }, { 0.196f * 3.0f, 0.223f * 3.0f, 0.286f * 3.0f, 1 }, 1600, 1600, 512, 0.1));
    //m_light2 = new PointLight({ 128, 200, 0, 1 }, { 0.196f * 3.0f, 0.223f * 3.0f, 0.286f * 3.0f, 1 }, 1600, 1600);

    m_tree = new Octree(m_brushes);
    for (auto light : m_lights)
    {
        auto box = ((PointLight*)light)->getBounds();
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

void Scene::addRigidBodies(GameManager* gameManager)
{
    for (u32 i = 0; i < m_physicsMeshes.size(); i++)
    {

    }
}

void Scene::generateShadowMaps()
{
    for (auto light : m_lights)
    {
        light->renderShadowMap(this);
    }
    //m_light2->renderShadowMap(this);
}

void Scene::initEntities()
{
    m_gameObjects.push_back(new Player(XMFLOAT3(0, 128, 0)));
}

void Scene::draw()
{
    m_shader->use();
    m_camera->update();

    Graphics.setViewMatrix(m_camera->getView());
    Graphics.setCameraPos(m_camera->getPosition());

    renderViewModels();
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
    return true;
}