#pragma once
#include "GTypes.h"

#include "IManager.h"

#include "PxPhysicsAPI.h"
#include "cooking/PxCooking.h"


class Scene;

using namespace physx;

class GameManager : public IManager
{
public:
    virtual bool start();
    virtual bool shutDown();

    void draw();
    void update();

    Scene* getScene();
    u64 getTime();

    PxPhysics* getPhysics();
    PxScene* getPxScene();
    PxCooking* getCooking();
    PxControllerManager* getControllers();

private:
    Scene* m_scene;

    u64 m_time;
    u64 m_acc = 0;

    PxFoundation* m_foundation;
    PxPhysics* m_physics;
    PxScene* m_pxScene;

    PxDefaultAllocator m_allocator;
    PxDefaultErrorCallback m_errCallback;

    PxDefaultCpuDispatcher* m_dispatcher;
    PxControllerManager* m_controllers;

    PxPvd* m_pvd;

    PxCooking* m_cooking;

    void startPhysX();
};

extern GameManager Game;