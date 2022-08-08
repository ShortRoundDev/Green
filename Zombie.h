#pragma once

#include "Actor.h"
#include "Animator.h"

class Zombie : public Actor
{
public:
    Zombie(XMFLOAT3 pos);
    ~Zombie();

    void update();
    void draw(Shader* shaderOverride = nullptr);
    void think();

private:
    Animator* m_animator;
};

