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

    void getFinalMatrix(std::vector<XMMATRIX>& finalMatrix);
    std::vector<AnimationJoint>& getJoints();
    u8* getBoneHierarchy();

private:
    u8* m_boneHierarchy;
    std::vector<AnimationJoint> m_joints;

};

