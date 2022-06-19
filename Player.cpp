#include "Player.h"

#include "Logger.h"
#include "GameManager.h"
#include "GraphicsManager.h"
#include "Scene.h"
#include "SingleActorExclusionFilter.h"
#include "PlayerBehaviorCallback.h"
#include "Camera.h"
#include "PointLight.h"

#include "MapFile_Parse.h"

#include "MeshEntity.h"
#include "AmbientLightVolume.h"

static ::Logger logger = CreateLogger("Player");

const PxControllerFilters filters;

Player* Player::Create(MF_Entity* entity)
{
    MF_Vector3 pos;
    if (MF_GetAttributeVec3(entity, "origin", &pos))
    {
        return new Player(MF3_TO_XM3(pos));
    }
    return nullptr;
}


Player::Player(XMFLOAT3 pos) :
    GameObject(pos, TYPE_ID(Player), &Game),
    m_lastPos(pos.x, pos.y, pos.z)
{
    auto material = Game.getPhysics()->createMaterial(0.5, 0.5, 0.6);
    PxCapsuleControllerDesc desc;
    //PxControllerDesc
    desc.radius = 16.0f;
    m_radius = desc.radius;

    desc.height = 55.0f;
    m_height = desc.height;
    
    desc.invisibleWallHeight = 3.0;
    desc.climbingMode = PxCapsuleClimbingMode::eEASY;
    desc.material = material;
    desc.behaviorCallback = new PlayerBehaviorCallback();
    //desc.call
    m_controller = Game.getControllers()->createController(desc);
    m_controller->setPosition(PxExtendedVec3(pos.x, pos.y, pos.z));
    if (!m_controller)
    {
        logger.err("Invalid! %d", desc.isValid());
        return;
    }

    m_filter = new SingleActorExclusionFilter(m_controller->getActor());

    ///// Move constants /////
    XMFLOAT3 minMove = XMFLOAT3(-3.0f, -9999, -3.0f);
    XMFLOAT3 maxMove = XMFLOAT3(3.0f, 9999, 3.0f);

    m_minMove = XMLoadFloat3(&minMove);
    m_maxMove = XMLoadFloat3(&maxMove);

    XMFLOAT3 friction = XMFLOAT3(0.8f, 0.0f, 0.8f);
    m_friction = XMLoadFloat3(&friction);

    XMFLOAT3 jump = XMFLOAT3(0.0f, 5.0f, 0.0f);
    m_jump = XMLoadFloat3(&jump);

    XMFLOAT3 gravity = XMFLOAT3(0.0f, -0.25f, 0.0f);
    m_gravity = XMLoadFloat3(&gravity);
}

Player::~Player()
{

}

void Player::update()
{
    setVectors();
    checkFloor();

    PxExtendedVec3 pos = m_controller->getPosition();
    XMFLOAT3 moveVec = XMFLOAT3(pos.x - m_lastPos.x, pos.y - m_lastPos.y, pos.z - m_lastPos.z);
    XMVECTOR moveVecV = XMLoadFloat3(&moveVec);

    if (m_onGround)
    {
        moveVecV = XMVectorMultiply(moveVecV, m_friction);
    }
    else
    {
        moveVecV = XMVectorAdd(moveVecV, m_gravity);
    }
    
    inputMove(moveVecV);

    XMStoreFloat3(&moveVec, moveVecV);
    auto move = PxVec3(moveVec.x, moveVec.y, moveVec.z);
    logger.info("%f, %f, %f", move.x, move.y, move.x);
    m_lastPos = pos;
    m_controller->move(move, 0.0f, 0, filters);
    
    pos = m_controller->getPosition();
    Game.getScene()->getCamera()->setPosition(pos.x, pos.y + 16.0f, pos.z);
}

void Player::setVectors()
{
    ///// FORWARD /////
    auto view = Game.getScene()->getCamera()->getView();
    view = XMMatrixTranspose(view);
    auto look = XMFLOAT3(
        view.r[2].m128_f32[0],
        0,
        view.r[2].m128_f32[2]
    );    
    m_forward = XMLoadFloat3(&look);
    m_forward = XMVector3Normalize(m_forward);

    ///// UP /////
    auto up = m_controller->getUpDirection();
    auto upF3 = XMFLOAT3(up.x, up.y, up.z);
    m_up = XMLoadFloat3(&upF3);

    ///// RIGHT /////
    m_right = XMVector3Cross(m_up, m_forward);
}

void Player::inputMove(XMVECTOR& moveVec)
{
    if (Graphics.keyDown('W'))
    {
        moveVec = XMVectorAdd(XMVectorScale(m_forward, 0.7f), moveVec);
    }

    if (Graphics.keyDown('S'))
    {
        moveVec = XMVectorAdd(XMVectorScale(m_forward, -0.7f), moveVec);
    }
    
    if (m_onGround && Graphics.keyDownEdge(32))
    {
        moveVec = XMVectorAdd(m_jump, moveVec);
    }

    XMFLOAT3 move3;
    XMStoreFloat3(&move3, moveVec);
    
    XMFLOAT2 moveXZ = XMFLOAT2(move3.x, move3.z);
    XMVECTOR moveXZV = XMLoadFloat2(&moveXZ);
    
    XMVECTOR lengthV = XMVector2Length(moveXZV);

    XMFLOAT2 length2;
    XMStoreFloat2(&length2, lengthV);

    moveXZV = XMVectorScale(XMVector2Normalize(moveXZV), std::min(3.0f, std::max(-3.0f, length2.x)));
    XMStoreFloat2(&moveXZ, moveXZV);
    move3.x = moveXZ.x;
    move3.z = moveXZ.y;

    moveVec = XMLoadFloat3(&move3);
    //moveVec = XMVectorClamp(moveVec, m_minMove, m_maxMove);
}

void Player::_update()
{
    thread_local static AmbientLightVolume* myLight;
    thread_local static f32 t = 0.0f;
    thread_local static DirectionalLight targetLightFrame, prevLightFrame;

    checkFloor();
    auto view = Game.getScene()->getCamera()->getView();
    view = XMMatrixTranspose(view);
    auto look = XMFLOAT3(
        view.r[2].m128_f32[0],
        0,
        view.r[2].m128_f32[2]
    );
    XMVECTOR _look = XMLoadFloat3(&look);
    _look = XMVector3Normalize(_look);
    ::XMStoreFloat3(&look, _look);

    XMFLOAT3 walk = XMFLOAT3(0.0f, 0.0f, 0.0f);

    if (!m_onGround)
    {
        m_move.y -= 0.25;
    }

    auto pos = m_controller->getPosition();

    Game.getScene()->getCamera()->setPosition(pos.x, pos.y + 16.0f, pos.z);

    if (m_onGround)
    {
        //friction
        XMFLOAT3 moveXZ = XMFLOAT3(m_move.x, 0, m_move.z);
        XMVECTOR moveV = XMLoadFloat3(&moveXZ);
        moveV = XMVectorScale(moveV, 0.8f);
        XMStoreFloat3(&moveXZ, moveV);

        m_move.x = moveXZ.x;
        m_move.z = moveXZ.z;
    }

    if (Graphics.keyDown('W'))
    {
        walk.x += look.x * 0.7f;
        walk.z += look.z * 0.7f;
    }

    if (Graphics.keyDownEdge('E'))
    {
        Graphics.setMouseLook(!Graphics.getMouseLook());
    }

    if (Graphics.keyDown('A'))
    {
        XMFLOAT3 lookPlane(look.x, 0, look.z);
        XMVECTOR lookV = XMLoadFloat3(&lookPlane);
        
        XMFLOAT3 up(0, 1, 0);
        XMVECTOR upV = XMLoadFloat3(&up);
        
        XMVECTOR leftV = XMVector3Cross(lookV, upV);
        XMFLOAT3 left;
        XMStoreFloat3(&left, leftV);

        walk.x += left.x * 0.7f;
        walk.z += left.z * 0.7f;
    }

    if (Graphics.keyDown('D'))
    {
        XMFLOAT3 lookPlane(look.x, 0, look.z);
        XMVECTOR lookV = XMLoadFloat3(&lookPlane);

        XMFLOAT3 down(0, -1, 0);
        XMVECTOR downV = XMLoadFloat3(&down);

        XMVECTOR rightV = XMVector3Cross(lookV, downV);
        XMFLOAT3 right;
        XMStoreFloat3(&right, rightV);

        walk.x += right.x * 0.7f;
        walk.z += right.z * 0.7f;
    }

    if (Graphics.keyDown('S'))
    {
        walk.x += -look.x * 0.7f;
        walk.z += -look.z * 0.7f;
    }

    if (Graphics.keyDown('U'))
    {
        Graphics.m_gBuffer.pointradius += 0.0001f;
        logger.info("%f", Graphics.m_gBuffer.pointradius);
    }

    if (Graphics.keyDown('J') && Graphics.m_gBuffer.pointradius > 0.0f)
    {
        Graphics.m_gBuffer.pointradius -= 0.0001f;
        logger.info("%f", Graphics.m_gBuffer.pointradius);
    }

    XMVECTOR walkV = XMLoadFloat3(&walk);
    XMVECTOR lenV = XMVector3Length(walkV);
    XMFLOAT3 len;

    XMStoreFloat3(&len, lenV);
    walkV = XMVector3ClampLength(walkV, 0.0f, 3.0f);
    
    XMStoreFloat3(&walk, walkV);

    if (m_onGround)
    {
        m_move.x += walk.x;
        m_move.z += walk.z;
    }
    else
    {
        /*if (walk.x != 0.0f)
        {
            m_move.x = walk.x;
        }
        if (walk.z != 0.0f)
        {
            m_move.z = walk.z;
        }*/
    }

    std::vector<MeshEntity*> lights;
    Game.getScene()->getTree()->queryType(m_pos, lights, AMBIENT_LIGHT_VOLUME);
    if (lights.size() > 0)
    {
        AmbientLightVolume* light = (AmbientLightVolume*)lights[0];

        if (myLight != light)
        {
            if (myLight == NULL)
            {
                prevLightFrame = light->getLightDesc();
                targetLightFrame = light->getLightDesc();
            }
            else
            {
                prevLightFrame = targetLightFrame;
                targetLightFrame = light->getLightDesc();
            }

            myLight = light;
            t = 0.0f;
        }

        XMVECTOR    prevAmbientA = XMLoadFloat4(&prevLightFrame.ambientA), targetAmbientA = XMLoadFloat4(&targetLightFrame.ambientA),
                    prevAmbientB = XMLoadFloat4(&prevLightFrame.ambientB), targetAmbientB = XMLoadFloat4(&targetLightFrame.ambientB),
                    prevAmbientDirection    = XMLoadFloat4(&prevLightFrame.ambientDirection),
                    targetAmbientDirection  = XMLoadFloat4(&targetLightFrame.ambientDirection);

        XMVECTOR lerpedAmbientAV = XMVectorLerp(prevAmbientA, targetAmbientA, t);
        XMVECTOR lerpedAmbientBV = XMVectorLerp(prevAmbientB, targetAmbientB, t);
        XMVECTOR lerpedAmbientDirV = XMVectorLerp(prevAmbientDirection, targetAmbientDirection, t);

        XMFLOAT4 lerpedAmbientA, lerpedAmbientB, lerpedAmbientDir;
        XMStoreFloat4(&lerpedAmbientA, lerpedAmbientAV);
        XMStoreFloat4(&lerpedAmbientB, lerpedAmbientBV);
        XMStoreFloat4(&lerpedAmbientDir, lerpedAmbientDirV);

        f32 lerpedHardness = prevLightFrame.hardness + ((targetLightFrame.hardness - prevLightFrame.hardness) * t);

        DirectionalLight lightDesc;
        Graphics.m_gBuffer.sun.ambientA = lerpedAmbientA;
        Graphics.m_gBuffer.sun.ambientB = lerpedAmbientB;
        Graphics.m_gBuffer.sun.ambientDirection = lerpedAmbientDir;
        Graphics.m_gBuffer.sun.hardness = lerpedHardness;
    }

    if (t < 1.0f)
    {
        t += 0.02f;
        if (t > 1.0f)
        {
            t = 1.0f;
        }
    }

    if (m_onGround && Graphics.keyDown(32))
    {
        m_move.y = 5;
    }

    if (Graphics.keyDownEdge('B'))
    {
        Game.getScene()->lights ^= true;
    }
    
    m_controller->move(m_move, 0.1f, 0, filters);

    pos = m_controller->getPosition();
    m_pos.x = pos.x;
    m_pos.y = pos.y;
    m_pos.z = pos.z;
}

void Player::draw()
{

}

void Player::checkFloor()
{
    PxQueryFilterData fd;
    fd.flags |= PxQueryFlag::eANY_HIT;
    fd.flags |= PxQueryFlag::ePREFILTER;
    PxExtendedVec3 foot = m_controller->getFootPosition();
    PxVec3 origin = PxVec3(foot.x, foot.y, foot.z);

    PxOverlapBuffer hit;
    float radius = m_radius - 1.0f;
    PxSphereGeometry sphere = PxSphereGeometry(radius);
    PxTransform pose = PxTransform(origin + (PxVec3(0, m_radius - 2.0f, 0)));

    if (Game.getPxScene()->overlap(sphere, pose, hit, fd, m_filter))
    {
        if (!m_onGround)
        {
            m_move.y = 0;
        }
        m_onGround = true;
    }
    else
    {
        m_onGround = false;
    }
}