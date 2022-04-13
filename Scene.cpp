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

static ::Logger logger = CreateLogger("Scene");

Scene::Scene(std::string fileName, GameManager* gameManager)
{
    initSceneTextures();
    if (!Mesh::createFromFile(
        fileName,
        &m_brushes, &m_meshCount,
        &m_physicsMeshes, &m_physicsMeshCount,
        gameManager
    ))
    {
        logger.err("Failed to load meshes!");
        return;
    }
    addRigidBodies(gameManager);

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
    for (u32 i = 0; i < m_physicsMeshCount; i++)
    {
        Vector3 position(0, 0, 0);
        Quaternion identity = Quaternion::identity();
        Transform transform(position, identity);

        RigidBody* body = gameManager->getPhysicsWorld()->createRigidBody(transform);
        Collider* collider = body->addCollider(m_physicsMeshes[i], transform);
        body->setType(BodyType::STATIC);
        m_walls.push_back(body);
    }
}

void Scene::generateShadowMaps()
{
   // m_light->renderShadowMap(this);
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
    for (u32 i = 0; i < m_meshCount; i++)
    {
        m_brushes[i].draw();
    }
}

void Scene::update()
{

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