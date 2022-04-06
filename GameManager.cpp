#include "GameManager.h"

#include "Scene.h"

bool GameManager::start()
{
    m_scene = new Scene("Boxes.obj");
    return true;
}

bool GameManager::shutDown()
{
    return true;
}

void GameManager::draw()
{
    m_scene->draw();
}

Scene* GameManager::getScene()
{
    return m_scene;
}