#include "GameManager.h"

#include "GraphicsManager.h"

#include "MapFile.h"

#include "Scene.h"

#include "Player.h"

constexpr u64 timeStepU = 8;
constexpr f32 timeStepF = (f32)timeStepU / 1000.0f;

bool GameManager::start()
{
    startPhysX();
    MF_Init();

    m_scene = new Scene("School", this);

    m_scene->generateShadowMaps();
    //m_scene->initEntities();
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

void GameManager::update(u64 time, f32 diff)
{
    m_time = time;
    m_diff = diff;
    m_pxScene->simulate(diff);
    m_pxScene->fetchResults(true);
    m_scene->update();
}

Scene* GameManager::getScene()
{
    return m_scene;
}

u64 GameManager::getTime()
{
    return m_time;
}

f32 GameManager::getDiff()
{
    return m_diff;
}

Player* GameManager::getPlayer()
{
    return m_player;
}

void GameManager::setPlayer(Player* player)
{
    m_player = player;
}

void GameManager::startPhysX()
{
    //base
    m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errCallback);
    
    //debugger
    m_pvd = PxCreatePvd(*m_foundation);
    PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
    m_pvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

    //Physics + Scene
    m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, PxTolerancesScale(), true, m_pvd);
    PxSceneDesc sceneDesc(m_physics->getTolerancesScale());
    sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f); // maybe x32
    m_dispatcher = PxDefaultCpuDispatcherCreate(2);
    sceneDesc.cpuDispatcher = m_dispatcher;
    sceneDesc.filterShader = PxDefaultSimulationFilterShader;
    m_pxScene = m_physics->createScene(sceneDesc);

    PxPvdSceneClient* pvdClient = m_pxScene->getScenePvdClient();
    if (pvdClient)
    {
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
        pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
    }

    PxCookingParams params(m_physics->getTolerancesScale());
    m_cooking = PxCreateCooking(PX_PHYSICS_VERSION, *m_foundation, params);

    m_controllers = PxCreateControllerManager(*m_pxScene);
}

PxPhysics* GameManager::getPhysics()
{
    return m_physics;
}

PxScene* GameManager::getPxScene()
{
    return m_pxScene;
}

PxCooking* GameManager::getCooking()
{
    return m_cooking;
}

PxControllerManager* GameManager::getControllers()
{
    return m_controllers;
}