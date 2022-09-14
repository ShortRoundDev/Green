#include "Zombie.h"

#include "GraphicsManager.h"

#include "GTypes.h"
#include "Scene.h"
#include "Player.h"

#include "Logger.h"

static ::Logger logger = CreateLogger("Zombie");

const PxControllerFilters filters;

Zombie::Zombie(XMFLOAT3 pos) : Actor(
    "RbgBox.gltf",
    pos,
    TYPE_ID(Zombie)
)
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

    m_animator = new Animator(new Animation("RbgBox.gltf", m_mesh));
}

Zombie::~Zombie()
{

}

void Zombie::update()
{
    m_animator->update(1.0f);
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
    XMMATRIX transform = XMMatrixIdentity();

    XMVECTOR pos = XMLoadFloat3(&m_pos);
    XMFLOAT3 one = XMFLOAT3(16.0f, 16.0f, 16.0f);
    XMVECTOR oneV = XMLoadFloat3(&one);

    transform = XMMatrixTransformation(XMVectorZero(), XMVectorZero(), oneV, XMVectorZero(), XMVectorZero(), pos);

    Shader* shader = shaderOverride ? shaderOverride : m_shader;
    shader->use();
    shader->bindModelMatrix(transform);
    //shader->bindModelMatrix(transform, &(m_animator->getFinalBoneMatrices()), (u32)m_animator->getFinalBoneMatrices().size());
    auto bones = m_animator->getFinalBoneMatrices();
    m_mesh->draw();

}

void Zombie::think()
{

}