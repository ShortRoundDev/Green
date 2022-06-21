#pragma once
#include "GameObject.h"

class Mesh;
class Shader;

class SimpleGameObject : public GameObject
{
public:
    SimpleGameObject(XMFLOAT3 pos);
    ~SimpleGameObject();

    void update();
    void draw();

    Mesh* m_mesh;
    Shader* m_shader;
};

