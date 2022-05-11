#include "Player.h"

#include "Logger.h"
#include "GameManager.h"
#include "GraphicsManager.h"
#include "Scene.h"
#include "SingleActorExclusionFilter.h"
#include "Camera.h"

static ::Logger logger = CreateLogger("Player");

const PxControllerFilters filters;

Player::Player(XMFLOAT3 pos) :
    GameObject(pos, TYPE_ID(Player), &Game)
{
    auto material = Game.getPhysics()->createMaterial(0.5, 0.5, 0.6);
    PxCapsuleControllerDesc desc;
    //PxControllerDesc
    desc.radius = 32.0f;
    desc.height = 48.0f;
    desc.material = material;
    m_controller = Game.getControllers()->createController(desc);
    m_controller->setPosition(PxExtendedVec3(0, 128, 0));
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
    logger.info("Onfloor: %d", m_onGround);
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

    if (!m_onGround)
    {
        m_move.y -= 0.25;
    }

    auto pos = m_controller->getPosition();

    Game.getScene()->getCamera()->setPosition(pos.x, pos.y, pos.z);

    if (m_onGround)
    {
        m_move.x = 0;
        m_move.z = 0;
    }

    if (Graphics.keyDown('W'))
    {
        m_move.x = look.x * 3;
        m_move.z = look.z * 3;
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