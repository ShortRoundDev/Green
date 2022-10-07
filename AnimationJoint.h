#pragma once

#include "AnimationChannel.h"

#include <DirectXMath.h>

using namespace DirectX;

class AnimationJoint
{
public:
    AnimationJoint(u32 boneId, XMMATRIX inverseBindMatrix, AnimationChannel animationChannel, XMMATRIX localMatrix);
    ~AnimationJoint();

    XMMATRIX getTransformation(f32 time);
    XMMATRIX getInverseBind();
    XMMATRIX getLocalMatrix();
    XMMATRIX getSkinMatrix(XMMATRIX parentSkinMatrix);
    f32 getMaxTime();

private:
    XMMATRIX m_inverseBindMatrix;
    AnimationChannel m_animationChannel;
    u32 m_boneId;
    XMMATRIX m_localMatrix;
    f32 m_maxTime = -1.0f;
};

