#pragma once

#include "AnimationChannel.h"

#include <DirectXMath.h>

using namespace DirectX;

class AnimationJoint
{
public:
    AnimationJoint(u32 boneId, XMMATRIX inverseBindMatrix, AnimationChannel animationChannel, XMMATRIX localMatrix, XMFLOAT3 pos);

    ~AnimationJoint();

    XMMATRIX getTransformation(f32 time);
    XMMATRIX getInverseBind();
    XMMATRIX getLocalMatrix();
    XMMATRIX getSkinMatrix(XMMATRIX parentSkinMatrix);
    f32 getMaxTime();
    const XMFLOAT3& getPos();

private:
    XMMATRIX m_inverseBindMatrix;
    AnimationChannel m_animationChannel;
    u32 m_boneId;
    XMMATRIX m_localMatrix;
    f32 m_maxTime = -1.0f;
    XMFLOAT3 m_pos;
};

