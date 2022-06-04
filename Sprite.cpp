#include "Sprite.h"

#include "GraphicsManager.h"
#include "GameManager.h"
#include "Mesh.h"
#include "Logger.h"
#include "Shader.h"

#include <vector>

static Logger logger = CreateLogger("Sprite");

Sprite::Sprite(std::string textureName, XMFLOAT3 position, XMFLOAT4 color) :
    m_shader(nullptr),
    m_mesh(nullptr),
    m_texture(nullptr),
    m_cBuffer({
        XMFLOAT4(position.x, position.y, position.z, 1.0f),
        color,
        XMFLOAT2(32.0f, 32.0f)
    })
{
    std::vector<Mesh*> meshes;
    Mesh::createFromFile("Sprite.obj", meshes, &Game);
    if (meshes.size() == 0)
    {
        logger.err("Failed to load sprite obj!");
        m_mesh = nullptr;
        return;
    }

    m_mesh = meshes[0];
    m_texture = Graphics.getTexture(textureName);
    m_mesh->setTexture(m_texture);
    m_shader = Graphics.getShader(L"Sprite");
}

Sprite::~Sprite()
{

}

SpriteBuffer Sprite::getCbuffer()
{
    return m_cBuffer;
}

void Sprite::draw()
{
    if (!m_shader || !m_mesh)
    {
        return;
    }
    m_shader->use();
    m_shader->bindCBuffer(&m_cBuffer);
    m_mesh->draw();
}