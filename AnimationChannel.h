#pragma once

#include "GTypes.h"

#include <DirectXMath.h>

#include <vector>

using namespace DirectX;

struct AnimationKeyFrame
{
    f32 timeStamp;
    XMVECTOR value;

    AnimationKeyFrame() :
        timeStamp(0),
        value(g_XMOne)
    {

    }

    AnimationKeyFrame(f32 timeStamp, XMVECTOR value) :
        timeStamp(timeStamp),
        value(value)
    {
    }
};

class AnimationChannel
{
public:
    AnimationChannel(
        std::vector<AnimationKeyFrame>& translation,
        std::vector<AnimationKeyFrame>& scale,
        std::vector<AnimationKeyFrame>& rotation
    );
    ~AnimationChannel();

    void getBoneTransform(XMMATRIX& transform, f32 timeStamp);

private:
    std::vector<AnimationKeyFrame> m_translation;
    std::vector<AnimationKeyFrame> m_scale;
    std::vector<AnimationKeyFrame> m_rotation;
};

