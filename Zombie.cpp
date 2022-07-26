#include "Zombie.h"

#include "GraphicsManager.h"

#include "GTypes.h"

Zombie::Zombie(XMFLOAT3 pos) : Actor(
    "zombie.gltf",
    pos,
    nullptr,
    TYPE_ID(Zombie)
)
{
    m_shader = Graphics.getShader(L"World");
}

Zombie::~Zombie()
{

}

void Zombie::update()
{

}

void Zombie::draw()
{
    XMMATRIX transform = XMMatrixIdentity();

    XMVECTOR pos = XMLoadFloat3(&m_pos);
    XMFLOAT3 one = XMFLOAT3(32.0f, 32.0f, 32.0f);
    XMVECTOR oneV = XMLoadFloat3(&one);

    transform = XMMatrixTransformation(XMVectorZero(), XMVectorZero(), oneV, XMVectorZero(), XMVectorZero(), pos);

    m_shader->use();
    m_shader->bindModelMatrix(transform);
    m_mesh->draw();

}

void Zombie::think()
{

}