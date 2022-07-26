#pragma once

#include "Actor.h"

class Zombie :public  Actor
{
public:
    Zombie(XMFLOAT3 pos);
    ~Zombie();

    void update();
    void draw();
    void think();

private:
};

