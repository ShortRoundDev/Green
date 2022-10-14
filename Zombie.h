#pragma once

#include "Actor.h"
#include "Hitbox.h"

#include <PxPhysicsAPI.h>
#include <DirectXMath.h>
using namespace DirectX;
using namespace physx;

class Zombie : public Actor
{
public:
    Zombie(XMFLOAT3 pos);
    ~Zombie();

    void update();
    void draw(Shader* shaderOverride = nullptr);
    void think();

private:
    std::vector<XMMATRIX> m_skeleton;
    std::vector<Hitbox> m_boneBoxes;

    void checkFloor();
    void setVectors();

    f32 m_radius;
    bool m_onGround;
    PxVec3                  m_move;
    PxExtendedVec3          m_lastPos;

    f32 m_rotation;
    f32 m_rotTarget;

    XMVECTOR m_forward;
    XMVECTOR m_right;
    XMVECTOR m_up;
    XMVECTOR m_minMove;
    XMVECTOR m_maxMove;
    XMVECTOR m_friction;
    XMVECTOR m_gravity;
    XMVECTOR m_stopDown;
    XMVECTOR m_jump;

    PxQueryFilterCallback* m_filter;

};

