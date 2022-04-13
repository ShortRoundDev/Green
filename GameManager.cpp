#include "GameManager.h"
#include "GraphicsManager.h"

#include "Scene.h"

constexpr u64 timeStepU = 16;
constexpr f32 timeStepF = (f32)timeStepU / 1000.0f;

bool GameManager::start()
{
    m_physicsWorld = m_physicsCommon.createPhysicsWorld();

    m_scene = new Scene("Boxes.obj", this);
    m_scene->generateShadowMaps();
    Graphics.swap();

    m_time = GetTickCount64();
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

void GameManager::update()
{
    u64 ticks = GetTickCount64();
    u64 delta = ticks - m_time;
    m_time = ticks;

    m_acc += delta;

    while (m_acc >= timeStepU)
    {
        m_physicsWorld->update(timeStepF);
        m_acc -= timeStepU; // this might cause problems later. Whatever
    }
}

Scene* GameManager::getScene()
{
    return m_scene;
}

PhysicsCommon* GameManager::getPhysicsCommon()
{
    return &m_physicsCommon;
}

PhysicsWorld* GameManager::getPhysicsWorld()
{
    return m_physicsWorld;
}

u64 GameManager::getTime()
{
    return m_time;
}