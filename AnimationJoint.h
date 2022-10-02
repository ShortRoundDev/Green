#pragma once

#include "AnimationChannel.h"

#include <DirectXMath.h>

using namespace DirectX;

class AnimationJoint
{
public:
    AnimationJoint(u32 boneId, XMMATRIX inverseBindMatrix, AnimationChannel animationChannel);
    ~AnimationJoint();

    XMMATRIX getTransformation();
    XMMATRIX getInverseBind();
    XMMATRIX getSkinMatrix(XMMATRIX parentSkinMatrix);


private:
    XMMATRIX m_inverseBindMatrix;
    AnimationChannel m_animationChannel;
    u32 m_boneId;
};

