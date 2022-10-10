#include "Hitbox.h"

#include "GraphicsManager.h"
#include "WavefrontMeshFactory.h"

#include "Shader.h"

#include "Logger.h"

thread_local static Logger logger = ::CreateLogger("Hitbox");

Hitbox::Hitbox(f32 radius, XMFLOAT4 color) :
    GameObject(XMFLOAT3(0, 0, 0), TYPE_ID(Hitbox)),
    m_color(color),
    m_radius(radius),
    m_mesh(nullptr),
    m_shader(nullptr)
{
    m_shader = Graphics.getShader(L"Hitbox");

    WavefrontMeshFactory factory("HitBall.obj");
    if (!factory.getStatus())
    {
        logger.err("Failed to load hitbox.obj!!");
        return;
    }
    MeshActor actor;
    factory.createMesh(actor);
    m_mesh = actor.mesh;
}

Hitbox::~Hitbox()
{

}

void Hitbox::update()
{

}

void Hitbox::draw(Shader* shaderOverride)
{
    static HitboxBuffer buffer;
    
    buffer.pos = m_pos;
    buffer.radius = m_radius;
    buffer.color = m_color;

    Graphics.setWireframe(true);

    Shader* shader = shaderOverride ? shaderOverride : m_shader;
    shader->use();

    shader->bindModelMatrix(XMMatrixIdentity());
    shader->bindCBuffer(&buffer);
    m_mesh->draw();

    Graphics.setWireframe(false);
}

void Hitbox::setPos(XMFLOAT3 pos)
{
    m_pos = pos;
}