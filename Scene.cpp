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
    //m_light = new PointLight({ 0, 64, 0, 1 }, { 1, 1, 1, 1 }, 800, 800);
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
   // m_light->renderShadowMap(this);
}

void Scene::initEntities()
{
    m_gameObjects.push_back(new Player(XMFLOAT3(0, 128, 0)));
}

void Scene::draw()
{
    //m_camera->move({0, 0, -1.0f});
    m_camera->update();
    Graphics.setViewMatrix(m_camera->getView());
    Graphics.setCameraPos(m_camera->getPosition());

    m_shader->use();
    renderMeshes();
}

void Scene::renderMeshes()
{
    //m_light->use(1);
    for (u32 i = 0; i < m_brushes.size(); i++)
    {
        if (m_brushes[i]->getTexture() == NULL)
        {
            logger.warn("%d: Texture is null", i);
        }
        m_brushes[i]->draw();
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

std::vector<GameObject*>& Scene::getGameObjects()
{
    return m_gameObjects;
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