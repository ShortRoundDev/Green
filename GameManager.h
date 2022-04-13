#pragma once
#include "GTypes.h"

#include "IManager.h"
#include "reactphysics3d/reactphysics3d.h"

using namespace reactphysics3d;

class Scene;

class GameManager : public IManager
{
public:
    virtual bool start();
    virtual bool shutDown();

    void draw();
    void update();

    Scene* getScene();
    PhysicsCommon* getPhysicsCommon();
    PhysicsWorld* getPhysicsWorld();
    u64 getTime();

private:
    Scene* m_scene;
    PhysicsCommon m_physicsCommon;
    PhysicsWorld* m_physicsWorld;

    u64 m_time;
    u64 m_acc = 0;
};

extern GameManager Game;