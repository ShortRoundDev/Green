#include "Zombie.h"

#include "GraphicsManager.h"
#include "SingleActorExclusionFilter.h"

#include "GTypes.h"
#include "Scene.h"
#include "Player.h"

#include "Logger.h"

static ::Logger logger = CreateLogger("Zombie");

const PxControllerFilters filters;

Zombie::Zombie(XMFLOAT3 pos) :
    Actor(
        "Zombie.gltf",
        pos,
        TYPE_ID(Zombie)
    ),
    m_boneBoxes(),
    m_radius(16.0f),
    m_lastPos(pos.x, pos.y, pos.z),
    m_rotation(0.0f)
{
    m_shader = Graphics.getShader(L"World");
    
    auto material = Game.getPhysics()->createMaterial(0.5f, 0.5f, 0.6f);

    PxCapsuleControllerDesc desc;
    desc.radius = m_radius;
    desc.height = 42.0f;
    desc.invisibleWallHeight = 3.0f;
    desc.climbingMode = PxCapsuleClimbingMode::eEASY;
    desc.material = material;
    m_controller = Game.getControllers()->createController(desc);
    m_controller->setPosition(PxExtendedVec3(pos.x, pos.y, pos.z));

    m_filter = new SingleActorExclusionFilter(m_controller->getActor());

    if (!m_controller)
    {
        logger.err("Failed to create capsule!");
    }
    m_boneBoxes.reserve(m_animations["Walk"]->getJoints().size());
    for (u32 i = 0; i < m_animations["Walk"]->getJoints().size(); i++)
    {
        m_boneBoxes.push_back(Hitbox(3.0f, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f)));
    }
}

Zombie::~Zombie()
{

}

void Zombie::update()
{
    setVectors();
    checkFloor();
    think();
}

void Zombie::draw(Shader* shaderOverride)
{
    static f32 time = 0.0f;
    time += 0.005f;
    XMMATRIX transform = XMMatrixIdentity();

    XMVECTOR pos = XMLoadFloat3(&m_pos);
    XMFLOAT3 one = XMFLOAT3(12.0f, 12.0f, 12.0f);
    XMVECTOR oneV = XMLoadFloat3(&one);

    auto controllerPos = m_controller->getPosition();

    transform = XMMatrixAffineTransformation(
        oneV,
        g_XMZero,
        XMQuaternionRotationRollPitchYaw(0.0f, m_rotation, 0.0f),
        XMVectorSet((f32)controllerPos.x, (f32)controllerPos.y, (f32)controllerPos.z, 0.0f)
    );

    Shader* shader = shaderOverride ? shaderOverride : m_shader;
    shader->use();

    m_skeleton.clear();
    m_animations["Walk"]->getFinalMatrix(m_skeleton, time);
    
    shader->bindModelMatrix(transform, &m_skeleton, (u32)m_skeleton.size());
    m_mesh->draw();

    Graphics.setDepthTest(false);
    for (u32 i = 0; i < m_animations["Walk"]->getJoints().size(); i++)
    {
        auto& m_hitbox = m_boneBoxes[i];

        auto pos = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        pos = XMVector4Transform(pos, XMMatrixInverse(nullptr, m_animations["Walk"]->getJoints()[i].getInverseBind()));
        pos = XMVector4Transform(pos, m_skeleton[i]);
        pos = XMVector4Transform(pos, transform);

        XMFLOAT3 _pos;
        XMStoreFloat3(&_pos, pos);
        m_hitbox.setPos(_pos);

        m_hitbox.draw();
    }
    Graphics.setDepthTest(true);
}

void Zombie::think()
{
    auto pos = m_controller->getPosition();
    XMVECTOR moveVec = XMVectorSet(
        pos.x - m_lastPos.x,
        pos.y - m_lastPos.y,
        pos.z - m_lastPos.z,
        1.0f
    );

    //friction
    if (m_onGround)
    {
        moveVec = XMVectorMultiply(moveVec, XMVectorSet(0.8f, 0.0f, 0.8f, 1.0f));
    }
    else
    {
        moveVec = XMVectorAdd(moveVec, XMVectorSet(0.0f, -0.25f, 0.0f, 0.0f));
    }

    // think here //

    auto playerPosVec = XMLoadFloat3(&Game.getPlayer()->getPos());
    auto myPosVec = XMLoadFloat3(&m_pos);
    auto diff = XMVectorSubtract(playerPosVec, myPosVec);
    diff = XMVector3Normalize(XMVectorSetY(diff, 0.0f)); // drop the y component

    m_rotTarget = -std::atan2f(XMVectorGetZ(diff), XMVectorGetX(diff)) + M_PI_2_F;
    
    auto rotTargetVec2 = XMVectorSet(XMVectorGetX(diff), XMVectorGetZ(diff), 0.0f, 0.0f);
    auto rotNowVec2 = XMVector2Normalize(
        XMVectorSet(std::sinf(m_rotation), std::cosf(m_rotation), 0.0f, 0.0f)
    );

    auto rotAngle = XMVectorGetX(XMVector2Cross(rotTargetVec2, rotNowVec2));

    logger.info("%05.2f", rotAngle);

    if (rotAngle > 0.0f)
    {
        m_rotation += 0.02f;
    }
    else
    {
        m_rotation -= 0.02f;
    }

    moveVec = XMVectorAdd(
        moveVec,
        XMVectorScale(
            XMVectorSet(std::sinf(m_rotation), 0.0f, std::cosf(m_rotation), 0.0f),
            0.1f
        )
    );

    //logger.info("%f", m_rotation);

    ////////////////
    auto move = PxVec3(XMVectorGetX(moveVec), XMVectorGetY(moveVec), XMVectorGetZ(moveVec));
    m_lastPos = m_controller->getPosition();
    m_controller->move(move, 0.0f, 0, filters);

    pos = m_controller->getPosition();
    m_pos = XMFLOAT3((f32)pos.x, (f32)pos.y, (f32)pos.z);
}

void Zombie::checkFloor()
{
    PxQueryFilterData fd;
    fd.flags |= PxQueryFlag::eANY_HIT;
    fd.flags |= PxQueryFlag::ePREFILTER;
    PxExtendedVec3 foot = m_controller->getFootPosition();
    PxVec3 origin = PxVec3((f32)foot.x, (f32)foot.y, (f32)foot.z);

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

void Zombie::setVectors()
{
    auto up = m_controller->getUpDirection();
    auto upF3 = XMFLOAT3(up.x, up.y, up.z);
    m_up = XMLoadFloat3(&upF3);
}