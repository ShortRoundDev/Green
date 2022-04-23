#include "GameManager.h"
#include "GraphicsManager.h"

#include "Scene.h"

constexpr u64 timeStepU = 16;
constexpr f32 timeStepF = (f32)timeStepU / 1000.0f;

bool GameManager::start()
{
    startPhysX();

    m_scene = new Scene("Boxes.obj", this);
    m_scene->generateShadowMaps();
    m_scene->initEntities();
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
        m_acc -= timeStepU; // this might cause problems later. Whatever
        m_pxScene->simulate(1.0f / 60.0f);
        m_pxScene->fetchResults(true);
    }
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