#pragma once

#include "GTypes.h"
#include "DirectXMath.h"

using namespace DirectX;

struct BoneInfo
{
    i32 id;
    XMMATRIX offset;
};
