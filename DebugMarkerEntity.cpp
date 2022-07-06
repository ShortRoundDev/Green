#include "DebugMarkerEntity.h"

#include "GTypes.h"

#include "Mesh.h"
#include "Shader.h"
#include "GraphicsManager.h"

DebugMarkerEntity::DebugMarkerEntity(XMFLOAT3 pos, f32 size) :
    DebugMarkerEntity(pos, size, {1.0f, 1.0f, 1.0f, 0.8f})
{

}

DebugMarkerEntity::DebugMarkerEntity(XMFLOAT3 pos, f32 size, XMFLOAT4 color) :
    GameObject(pos, TYPE_ID(DebugMarkerEntity)),
    m_size(size),
    m_color({ color }),
    m_shader(nullptr),
    m_mesh(nullptr)
{
    XMFLOAT3 max = XMFLOAT3(pos.x + size, pos.y + size, pos.z + size);
    Mesh::createBbox(AABB(pos, max), nullptr, m_mesh);
    m_shader = Graphics.getShader(L"Octree");
}

DebugMarkerEntity::~DebugMarkerEntity()
{

}

void DebugMarkerEntity::update()
{

}

void DebugMarkerEntity::draw()
{
    m_shader->use();
    m_shader->bindCBuffer(&m_color);
    m_mesh->draw();
}
