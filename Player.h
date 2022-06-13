#pragma once

#include "GameObject.h"

#include "PxPhysicsAPI.h"

using namespace physx;

struct MF_Entity;

class Player : public GameObject
{
public:
    static Player* Create(MF_Entity* entity);

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