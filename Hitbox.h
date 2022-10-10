#pragma once

#include "GTypes.h"
#include "GameObject.h"
#include "Mesh.h"
#include "HitboxBuffer.h"

#include <DirectXMath.h>

using namespace DirectX;

class Hitbox : public GameObject
{
public:
    Hitbox(f32 radius, XMFLOAT4 color);
    ~Hitbox();

    virtual void update();
    virtual void draw(Shader* shaderOverride = nullptr);

    void setPos(XMFLOAT3 pos);

private:
    XMFLOAT4 m_color;
    Mesh* m_mesh;
    f32 m_radius;
    Shader* m_shader;
};
