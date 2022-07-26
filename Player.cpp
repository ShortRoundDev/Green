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
#include "SimpleGameObject.h"
#include "OctreeNode.h"

static ::Logger logger = CreateLogger("Player");
const PxControllerFilters filters;

/***** CREATE ENTITY *****/
#pragma region CREATE_ENTITY
Player* Player::Create(MF_Entity* entity)
{
    MF_Vector3 pos;
    if (MF_GetAttributeVec3(entity, "origin", &pos))
    {
        return new Player(MF3_TO_XM3(pos));
    }
    return nullptr;
}
#pragma endregion

/***** CTOR/DTOR *****/
#pragma region CTOR_DTOR
Player::Player(XMFLOAT3 pos) :
    GameObject(pos, TYPE_ID(Player), &Game),
    m_lastPos(pos.x, pos.y, pos.z),
    m_filter(nullptr),
    m_forward(g_XMZero),
    m_friction(g_XMZero),
    m_gravity(g_XMZero),
    m_jump(g_XMZero),
    m_maxMove(g_XMZero),
    m_minMove(g_XMZero),
    m_onGround(false),
    m_right(g_XMZero),
    m_stopDown(g_XMZero),
    m_up(g_XMZero),
    m_path()
{
    m_path.reserve(4000);
    auto material = Game.getPhysics()->createMaterial(0.5f, 0.5f, 0.6f);
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
#pragma endregion

/***** UPDATE/DRAW *****/
#pragma region UPDATE_DRAW
void Player::update()
{
    setVectors();
    checkFloor();
    updateMoveThisFrame();
    updateCamera();
    updateAmbientLight();
}

void Player::draw()
{
    
//#ifndef _DEBUG
    return;
//#endif

    static auto node = Game.getScene()->getTree()->getNode(XMFLOAT3(452.664093f, 59.600002f, -513.489319f));
    static auto myNode = Game.getScene()->getTree()->getNode(m_pos);
    static bool found = false;

    if (m_path.size() == 0)
    {
        if (node->findPath(myNode, m_path))
        {
            found = true;
            logger.info("Found path");
        }
    }

    //Graphics.setWireframe(true);
    if (found)
    {
        auto next = myNode;
        while (next != node)
        {
            next = m_path[next];
            next->draw({ 1.0f, 0.0f, 1.0f, 1.0f });
        }
    }
    //Graphics.setWireframe(false);
}
#pragma endregion

/***** UPDATE METHODS *****/
#pragma region UPDATE_METHODS
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

void Player::updateMoveThisFrame()
{
    PxExtendedVec3 pos = m_controller->getPosition();
    XMFLOAT3 moveVec = XMFLOAT3(pos.x - m_lastPos.x, pos.y - m_lastPos.y, pos.z - m_lastPos.z);
    XMVECTOR moveVecV = XMLoadFloat3(&moveVec);

    if (m_onGround || m_gravityOff)
    {
        moveVecV = XMVectorMultiply(moveVecV, m_friction);
    }
    else if (!m_gravityOff)
    {
        moveVecV = XMVectorAdd(moveVecV, m_gravity);
    }

    inputMove(moveVecV);

    XMStoreFloat3(&moveVec, moveVecV);
    auto move = PxVec3(moveVec.x, moveVec.y, moveVec.z);

    m_lastPos = m_controller->getPosition();
    m_controller->move(move, 0.0f, 0, filters);

    pos = m_controller->getPosition();
    m_pos = XMFLOAT3(pos.x, pos.y, pos.z);

    return;
}

void Player::updateCamera()
{
    auto pos = m_controller->getPosition();
    Game.getScene()->getCamera()->setPosition(pos.x, pos.y + 16.0f, pos.z);
}

void Player::updateAmbientLight()
{
    std::vector<MeshEntity*> lights;
    Game.getScene()->getTree()->queryType(m_pos, lights, AMBIENT_LIGHT_VOLUME);
    if (lights.size() > 0)
    {
        AmbientLightVolume* light = (AmbientLightVolume*)lights[0];

        if (m_currentLight != light)
        {
            logger.info("New ambient");
            if (m_currentLight == nullptr)
            {
                m_prevLight     = light->getLightDesc();
                m_targetLight   = light->getLightDesc();
            }
            else
            {
                m_prevLight     = m_targetLight;
                m_targetLight   = light->getLightDesc();
            }

            m_currentLight  = light;
            m_lightT        = 0.0f;
        }

        XMVECTOR    prevAmbientA = XMLoadFloat4(&m_prevLight.ambientA), targetAmbientA = XMLoadFloat4(&m_targetLight.ambientA),
                    prevAmbientB = XMLoadFloat4(&m_prevLight.ambientB), targetAmbientB = XMLoadFloat4(&m_targetLight.ambientB),
                    prevAmbientDirection = XMLoadFloat4(&m_prevLight.ambientDirection),
                    targetAmbientDirection = XMLoadFloat4(&m_targetLight.ambientDirection);

        XMVECTOR lerpedAmbientAV = XMVectorLerp(prevAmbientA, targetAmbientA, m_lightT);
        XMVECTOR lerpedAmbientBV = XMVectorLerp(prevAmbientB, targetAmbientB, m_lightT);
        XMVECTOR lerpedAmbientDirV = XMVectorLerp(prevAmbientDirection, targetAmbientDirection, m_lightT);

        XMFLOAT4 lerpedAmbientA, lerpedAmbientB, lerpedAmbientDir;
        XMStoreFloat4(&lerpedAmbientA,      lerpedAmbientAV);
        XMStoreFloat4(&lerpedAmbientB,      lerpedAmbientBV);
        XMStoreFloat4(&lerpedAmbientDir,    lerpedAmbientDirV);

        f32 lerpedHardness = m_prevLight.hardness + ((m_targetLight.hardness - m_prevLight.hardness) * m_lightT);

        DirectionalLightBuffer lightDesc;
        Graphics.m_gBuffer.dirLight.ambientA         = lerpedAmbientA;
        Graphics.m_gBuffer.dirLight.ambientB         = lerpedAmbientB;
        Graphics.m_gBuffer.dirLight.ambientDirection = lerpedAmbientDir;
        Graphics.m_gBuffer.dirLight.hardness         = lerpedHardness;
    }

    if (m_lightT < 1.0f)
    {
        m_lightT += 0.02f;
        if (m_lightT > 1.0f)
        {
            m_lightT = 1.0f;
        }
    }
}

void Player::inputMove(XMVECTOR& moveVec)
{
    /***** MOVEMENT *****/
    if (Graphics.keyDown('W'))
    {
        moveVec = XMVectorAdd(XMVectorScale(m_forward, (m_onGround || m_gravityOff) ? 0.7f : 0.2f), moveVec);
    }

    if (Graphics.keyDown('S'))
    {
        moveVec = XMVectorAdd(XMVectorScale(m_forward, -((m_onGround || m_gravityOff) ? 0.7f : 0.2f)), moveVec);
    }

    if (Graphics.keyDown('D'))
    {
        moveVec = XMVectorAdd(XMVectorScale(m_right, (m_onGround || m_gravityOff) ? 0.7f : 0.2f), moveVec);
    }

    if (Graphics.keyDown('A'))
    {
        moveVec = XMVectorAdd(XMVectorScale(m_right, -((m_onGround || m_gravityOff) ? 0.7f : 0.2f)), moveVec);
    }
    /***** JUMPING (32 = space) *****/
    if ((m_onGround && Graphics.keyDownEdge(32)) || (m_gravityOff && Graphics.keyDown(32)))
    {
        if (m_gravityOff && Graphics.keyDown(16)) // 16 = Left Shift
        {
            moveVec = XMVectorAdd(XMVectorScale(m_jump, -1.0f), moveVec);
        }
        else
        {
            moveVec = XMVectorAdd(m_jump, moveVec);
        }
    }

    /***** DEBUG KEYS *****/
    if (Graphics.keyDownEdge('X'))
    {
        m_gravityOff ^= true;
    }

    if (Graphics.keyDownEdge('E'))
    {
        Graphics.setMouseLook(!Graphics.getMouseLook());
    }

    if (Graphics.keyDownEdge('P'))
    {
        
        /*auto foot = m_controller->getFootPosition();
        auto newPos = XMFLOAT3(
            static_cast<f32>(foot.x),
            static_cast<f32>(foot.y),
            static_cast<f32>(foot.z)
        );
        Game.getScene()->putGameObject(new SimpleGameObject(newPos));
        */

        logger.info("%f %f %f", m_pos.x, m_pos.y, m_pos.z);
    }

    /***** TREE VISUALIZATION *****/
    if (Graphics.keyDownEdge('Z'))
    {
        OctreeNode::SetShowGreen(!OctreeNode::IsShowGreen());
    }
    if (Graphics.keyDownEdge('R'))
    {
        OctreeNode::SetShowSize(OctreeNode::GetShowSize() - 1);
    }
    if (Graphics.keyDownEdge('F'))
    {
        OctreeNode::SetShowSize(OctreeNode::GetShowSize() + 1);
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
}
#pragma endregion