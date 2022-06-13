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
    GameObject(pos, TYPE_ID(Player), &Game)
{
    auto material = Game.getPhysics()->createMaterial(0.5, 0.5, 0.6);
    PxCapsuleControllerDesc desc;
    //PxControllerDesc
    desc.radius = 16.0f;
    desc.height = 55.0f;
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
}

Player::~Player()
{

}

void Player::update()
{
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
        logger.info("%f, %f, %f", m_pos.x, m_pos.y, m_pos.z);
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
    float radius = 31.0f;
    PxSphereGeometry sphere = PxSphereGeometry(radius);
    PxTransform pose = PxTransform(origin + (PxVec3(0, 30, 0)));

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