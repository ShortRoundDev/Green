#pragma once

#include "GameObject.h"

#include "PxPhysicsAPI.h"

using namespace physx;

class Player : public GameObject
{
public:
    Player(XMFLOAT3 pos);
    ~Player();

    virtual void update();
    virtual void draw();

private:
    PxController* m_controller;

    PxVec3 m_move;

    bool m_onGround;

    void checkFloor();

    PxQueryFilterCallback* m_filter;
};
