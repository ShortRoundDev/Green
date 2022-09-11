#pragma once

#include "GameObject.h"

#include "GlobalBuffer.h"

#include <PxPhysicsAPI.h>

#include <unordered_map>

using namespace physx;

struct MF_Entity;

class OctreeNode;
class AmbientLightVolume;

class Player : public GameObject
{
public:
    /***** CREATE ENTITY *****/
    static Player* Create(MF_Entity* entity);

    /***** CTOR/DTOR *****/
    Player(XMFLOAT3 pos);
    virtual ~Player();

    const XMFLOAT3& getPos();

    /***** UPDATE/DRAW *****/
    virtual void update();
    virtual void draw();

private:
    /***** FIELDS *****/
    /***** PHYSX *****/
    PxController*           m_controller;
    PxVec3                  m_move;
    PxExtendedVec3          m_lastPos;
    PxQueryFilterCallback*  m_filter;
    /***** PHYSICAL FIELDS *****/
    bool    m_onGround;
    f32     m_radius;
    f32     m_height;
    /***** MOTION AND VECTORS *****/
    XMVECTOR m_forward;
    XMVECTOR m_right;
    XMVECTOR m_up;
    XMVECTOR m_minMove;
    XMVECTOR m_maxMove;
    XMVECTOR m_friction;
    XMVECTOR m_gravity;
    XMVECTOR m_stopDown;
    XMVECTOR m_jump;
    /***** DEBUG *****/
    bool m_gravityOff = false;
    /***** AMBIENT LIGHT STATE *****/
    AmbientLightVolume* m_currentLight;
    f32 m_lightT;
    DirectionalLightBuffer m_targetLight;
    DirectionalLightBuffer m_prevLight;
    /***** nav *****/
    std::unordered_map<OctreeNode*, OctreeNode*> m_path;
    
    /***** UPDATE METHODS *****/
    void checkFloor();
    void setVectors();
    void updateMoveThisFrame();
    void updateCamera();
    void updateAmbientLight();
    void inputMove(XMVECTOR& moveVec);
};