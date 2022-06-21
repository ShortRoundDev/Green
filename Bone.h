#pragma once

#include "GTypes.h"
#include "KeyFrame.h"

#include <DirectXMath.h>

#include <vector>

using namespace DirectX;

class Bone
{
public:
    Bone()
    {

    }
    ~Bone()
    {

    }

    void interpolate(f32 t, XMMATRIX& transform)
    {

    }
    std::vector<KeyFrame> keyFrames;
};

