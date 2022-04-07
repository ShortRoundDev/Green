#include "Windows.h"

#include "SystemManager.h"
#include "LogManager.h"
#include "Logger.h"
#include "GraphicsManager.h"
#include "GameManager.h"
#include "Light.h"

SystemManager System;
GraphicsManager Graphics;
GameManager Game;

static Logger logger = CreateLogger("Program");

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    LogManager::getLogManager(); // implicitly calls start on load if not loaded yet

    logger.info("Starting up!");
    System.start();
    Graphics.start();
    Game.start();

    Light* light = new Light({0, 0, -256.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {0.0f, 0.0, 1.0f, 1.0f}, 1920, 1080);
    light->renderShadowMap(Game.getScene());
    Graphics.swap();

    while (!Graphics.update())
    {

        Graphics.clear();
        Graphics.bindGlobalBuffer();

        Game.draw();
        
        Graphics.swap();
    }
}