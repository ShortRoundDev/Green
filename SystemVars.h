#pragma once
#include "GTypes.h"

#include <string>

struct SystemVars
{
    u16 width = 1024;
    u16 height = 768;
    bool fullScreen = false;
    std::wstring title = L"Game";

    bool debugMode = false;
};

