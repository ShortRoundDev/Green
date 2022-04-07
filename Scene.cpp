#include "Scene.h"

#include "GraphicsManager.h"
#include "GTypes.h"
#include "Logger.h"
#include "Mesh.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera.h"

static Logger logger = CreateLogger("Scene");

Scene::Scene(std::string fileName)
{
    initSceneTextures();
    if (!Mesh::createFromFile(fileName, &m_brushes, &m_meshCount))
    {
        logger.err("Failed to load meshes!");
        return;
    }
    m_camera = new Camera();
    m_shader = Graphics.getShader(L"World");
}

Scene::~Scene()
{

}

void Scene::draw()
{
    //m_camera->move({0, 0, -1.0f});
    m_camera->update();
    Graphics.setViewMatrix(m_camera->getView());

    m_shader->use();
    renderMeshes();
}

void Scene::renderMeshes()
{
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