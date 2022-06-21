#pragma once

#include "GTypes.h"
#include "Bone.h"

#include "DirectXMath.h"

#include <vector>

using namespace DirectX;

class AnimationAction
{
public:
    AnimationAction(const std::vector<Bone>& bones)
    {

    }
    ~AnimationAction()
    {

    }

    f32 getStartTime();
    f32 getEndTime();

    void interpolate(f32 t, std::vector<XMMATRIX>& boneTransforms)
    {

    }
private:

    std::vector<Bone> bones;
};

