
#include "SystemManager.h"
#include "LogManager.h"
#include "Logger.h"
#include "GraphicsManager.h"
#include "GameManager.h"

#include <filesystem>

#include <iostream>

SystemManager System;
GraphicsManager Graphics;
GameManager Game;

constexpr u64 timeStepU = 16;
constexpr f32 timeStepF = (f32)timeStepU / 1000.0f;

static ::Logger logger = CreateLogger("Program");

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine,
    _In_ int nShowCmd
)
{
    u64 time = GetTickCount64();
    u64 acc = 0;

    LogManager::getLogManager(); // implicitly calls start on load if not loaded yet

    logger.info("Starting up!");
    System.start();
    std::wcout << std::filesystem::current_path() << std::endl;
    Graphics.start();
    std::wcout << std::filesystem::current_path() << std::endl;
    Game.start();
    std::wcout << std::filesystem::current_path() << std::endl;

    u64 beginFrameTime = 0;
    int frames = 0;
    bool quit = false;
    while (!quit)
    {
        Graphics.clear();
        Graphics.bindGlobalBuffer();

        u64 ticks = GetTickCount64();
        u64 delta = ticks - time;
        time = ticks;

        acc += delta;

        //while (acc >= timeStepU)
        //{
          //  acc -= timeStepU;
            Game.update(time, (f32)delta/1000.0f);
            quit = Graphics.update();
        //}

        Game.draw();
        Graphics.draw();
        Graphics.swap();
        frames++;
    }
}