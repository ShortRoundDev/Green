#include "Zombie.h"

#include "GraphicsManager.h"

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
    m_boneBoxes()
{
    m_shader = Graphics.getShader(L"World");
    
    auto material = Game.getPhysics()->createMaterial(0.5f, 0.5f, 0.6f);

    PxCapsuleControllerDesc desc;
    desc.radius = 16.0f;
    desc.height = 55.0f;
    desc.invisibleWallHeight = 3.0f;
    desc.climbingMode = PxCapsuleClimbingMode::eEASY;
    desc.material = material;
    m_controller = Game.getControllers()->createController(desc);
    m_controller->setPosition(PxExtendedVec3(pos.x, pos.y, pos.z));

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
    return;

    auto pPos = Game.getPlayer()->getPos();
    auto pPosV = XMLoadFloat3(&pPos);
    auto posV = XMLoadFloat3(&m_pos);

    auto diff = XMVectorSubtract(pPosV, posV);

    auto movePlane = XMFLOAT2(XMVectorGetX(diff), XMVectorGetZ(diff));
    auto movePlaneV = XMLoadFloat2(&movePlane);

    auto normV = XMVector2Normalize(movePlaneV);

    PxVec3 move = PxVec3(XMVectorGetX(normV), 0.0f, XMVectorGetY(normV));

    m_controller->move(move, 0.0f, 0, filters);

    PxExtendedVec3 pos = m_controller->getFootPosition();
    m_pos = XMFLOAT3((f32)pos.x, (f32)pos.y, (f32)pos.z);
}

void Zombie::draw(Shader* shaderOverride)
{
    static f32 time = 0.0f;
    time += 0.01f;
    XMMATRIX transform = XMMatrixIdentity();

    XMVECTOR pos = XMLoadFloat3(&m_pos);
    XMFLOAT3 one = XMFLOAT3(16.0f, 16.0f, 16.0f);
    XMVECTOR oneV = XMLoadFloat3(&one);

    transform = XMMatrixAffineTransformation(oneV, g_XMZero, g_XMZero, g_XMZero);

    Shader* shader = shaderOverride ? shaderOverride : m_shader;
    shader->use();

    m_skeleton.clear();
    m_animations["Walk"]->getFinalMatrix(m_skeleton, time);
    
    shader->bindModelMatrix(transform, &m_skeleton, (u32)m_skeleton.size());
    Graphics.setWireframe(true);
    m_mesh->draw();

    //transform = XMMatrixAffineTransformation(oneV, g_XMZero, g_XMZero, XMVectorSet(0, 0, 0, 1));

    for (u32 i = 0; i < m_animations["Walk"]->getJoints().size(); i++)
    {
        auto& m_hitbox = m_boneBoxes[i];

        auto pos = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
        pos = XMVector4Transform(pos, XMMatrixInverse(nullptr, m_animations["Walk"]->getJoints()[i].getInverseBind()));
        pos = XMVector4Transform(pos, transform);

        XMFLOAT3 _pos;
        XMStoreFloat3(&_pos, pos);
        m_hitbox.setPos(_pos);

        m_hitbox.draw();
    }
    Graphics.setWireframe(false);
}

void Zombie::think()
{

}