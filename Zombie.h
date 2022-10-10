#pragma once

#include "Actor.h"
#include "Hitbox.h"

#include <DirectXMath.h>
using namespace DirectX;

class Zombie : public Actor
{
public:
    Zombie(XMFLOAT3 pos);
    ~Zombie();

    void update();
    void draw(Shader* shaderOverride = nullptr);
    void think();

private:
    std::vector<XMMATRIX> m_skeleton;
    std::vector<Hitbox> m_boneBoxes;
};

