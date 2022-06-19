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
    virtual void _update();
    virtual void draw();

private:
    PxController* m_controller;

    PxVec3 m_move;

    bool m_onGround;
    f32 m_radius;
    f32 m_height;

    PxExtendedVec3 m_lastPos;

    XMVECTOR m_forward;
    XMVECTOR m_right;
    XMVECTOR m_up;

    XMVECTOR m_minMove;
    XMVECTOR m_maxMove;
    XMVECTOR m_friction;
    XMVECTOR m_gravity;
    XMVECTOR m_stopDown;
    XMVECTOR m_jump;

    void checkFloor();
    void setVectors();
    void inputMove(XMVECTOR& moveVec);

    PxQueryFilterCallback* m_filter;
};