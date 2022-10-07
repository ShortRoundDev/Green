#pragma once

#include "GTypes.h"
#include "AnimationJoint.h"

#include <DirectXMath.h>

#include <vector>

using namespace DirectX;

class AnimationSkeleton
{
public:
    AnimationSkeleton();
    AnimationSkeleton(u8* boneHierarchy, std::vector<AnimationJoint> joints);
    ~AnimationSkeleton();

    void getFinalMatrix(std::vector<XMMATRIX>& finalMatrix, f32 time);
    std::vector<AnimationJoint>& getJoints();
    u8* getBoneHierarchy();

    f32 getMaxTime();

private:
    u8* m_boneHierarchy;
    std::vector<AnimationJoint> m_joints;
    f32 m_maxTime = -1.0f;
};

