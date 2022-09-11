#pragma once
#include "GTypes.h"

#include "IManager.h"

#include <PxPhysicsAPI.h>
#include <cooking/PxCooking.h>

class Scene;
class Player;

using namespace physx;

class GameManager : public IManager
{
public:
    virtual bool start();
    virtual bool shutDown();

    void draw();
    void update(u64 time, f32 diff);

    Scene* getScene();
    u64 getTime();
    f32 getDiff();

    Player* getPlayer();
    void setPlayer(Player* player);

    PxPhysics* getPhysics();
    PxScene* getPxScene();
    PxCooking* getCooking();
    PxControllerManager* getControllers();

private:
    Scene* m_scene;

    u64 m_time;
    f32 m_diff;

    PxFoundation* m_foundation;
    PxPhysics* m_physics;
    PxScene* m_pxScene;

    PxDefaultAllocator m_allocator;
    PxDefaultErrorCallback m_errCallback;

    PxDefaultCpuDispatcher* m_dispatcher = nullptr;
    PxControllerManager* m_controllers = nullptr;

    PxPvd* m_pvd;

    PxCooking* m_cooking;

    Player* m_player;

    void startPhysX();
};

extern GameManager Game;