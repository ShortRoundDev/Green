#include "AnimationChannel.h"
#include <algorithm>

AnimationChannel::AnimationChannel(
    std::vector<AnimationKeyFrame>& translation,
    std::vector<AnimationKeyFrame>& scale,
    std::vector<AnimationKeyFrame>& rotation
) :
    m_translation(translation),
    m_scale(scale),
    m_rotation(rotation)
{

}

AnimationChannel::~AnimationChannel()
{

}

void AnimationChannel::getBoneTransform(XMMATRIX& transform, f32 timeStamp)
{
    u32 translationIdx = 0;
    for (translationIdx = 0; translationIdx < m_translation.size() && m_translation[translationIdx].timeStamp > timeStamp; translationIdx++);
    if (translationIdx > 0)
    {
        translationIdx--;
    }

    u32 scaleIdx = 0;
    for (scaleIdx = 0; scaleIdx < m_scale.size() && m_scale[scaleIdx].timeStamp > timeStamp; scaleIdx++);
    if (scaleIdx > 0)
    {
        scaleIdx--;
    }

    u32 rotationIdx = 0;
    for (rotationIdx = 0; rotationIdx < m_rotation.size() && m_rotation[rotationIdx].timeStamp > timeStamp; rotationIdx++);
    if (rotationIdx > 0)
    {
        rotationIdx--;
    }

    AnimationKeyFrame
        translate   = m_translation[translationIdx],
        scale       = m_scale[scaleIdx],
        rotation    = m_rotation[rotationIdx];

    transform = XMMatrixTranspose(XMMatrixTransformation(g_XMZero, g_XMZero, scale.value, g_XMZero, rotation.value, translate.value));
}