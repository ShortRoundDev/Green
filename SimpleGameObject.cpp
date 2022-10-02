#include "SimpleGameObject.h"

#include "Mesh.h"
#include "Shader.h"
#include "Scene.h"
#include "GraphicsManager.h"
#include "GameManager.h"
#include "GTypes.h"
#include "AmbientLightVolume.h"

#include "Logger.h"

static Logger logger = CreateLogger("SimpleGameObject");

SimpleGameObject::SimpleGameObject(XMFLOAT3 pos) :
    GameObject(pos, TYPE_ID(SimpleGameObject))
{
    m_shader = Graphics.getShader(L"World");

    //if (!Mesh::loadGltf("Ganon.gltf", &m_mesh, &Game))
    {
      //  logger.err("Failed to load ganon!");
        return;
    }
}

SimpleGameObject::~SimpleGameObject()
{

}

void SimpleGameObject::draw()
{
    /*std::vector<MeshEntity*> lights;
    Game.getScene()->getTree()->queryType(m_pos, lights, AMBIENT_LIGHT_VOLUME);
    if (lights.size() > 0)
    {
        AmbientLightVolume* light = (AmbientLightVolume*)lights[0];
        auto lightDesc = light->getLightDesc();

        Graphics.m_gBuffer.sun.ambientA = lightDesc.ambientA;
        Graphics.m_gBuffer.sun.ambientB = lightDesc.ambientB;
        Graphics.m_gBuffer.sun.ambientDirection = lightDesc.ambientDirection;
        Graphics.m_gBuffer.sun.hardness = lightDesc.hardness;
    }*/

    XMMATRIX transform = XMMatrixIdentity();

    XMVECTOR pos = XMLoadFloat3(&m_pos);
    XMFLOAT3 one = XMFLOAT3(1.0f, 1.0f, 1.0f);
    XMVECTOR oneV = XMLoadFloat3(&one);

    transform = XMMatrixTransformation(XMVectorZero(), XMVectorZero(), oneV, XMVectorZero(), XMVectorZero(), pos);
    
    m_shader->use();
    m_shader->bindModelMatrix(transform);
    m_mesh->draw();
}

void SimpleGameObject::update()
{
}