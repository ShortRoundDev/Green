#pragma once
#include "IManager.h"

class Scene;

class GameManager : public IManager
{
public:
    virtual bool start();
    virtual bool shutDown();

    void draw();

    Scene* getScene();

private:
    Scene* m_scene;
};

extern GameManager Game;