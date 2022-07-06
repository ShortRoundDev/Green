#pragma once
#include "GameObject.h"

#include "WireframeDebugColor.h"

class Mesh;
class Shader;

class DebugMarkerEntity : public GameObject
{
public:
    DebugMarkerEntity(XMFLOAT3 pos, f32 size);
    DebugMarkerEntity(XMFLOAT3 pos, f32 size, XMFLOAT4 color);
    ~DebugMarkerEntity();

    virtual void update();
    virtual void draw();
private:
    Mesh* m_mesh;
    WireframeDebugColor m_color;
    f32 m_size;
    Shader* m_shader;
};

