#include "AnimationChannel.h"
#include <algorithm>

#include <map>
#include <string>

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
    for (scaleIdx = 0; scaleIdx < m_scale.size() && m_scale[scaleIdx].timeStamp < timeStamp; scaleIdx++);
    if (scaleIdx > 0)
    {
        scaleIdx--;
    }

    u32 rotationIdx = 0;
    for (rotationIdx = 0; rotationIdx < m_rotation.size() && m_rotation[rotationIdx].timeStamp < timeStamp; rotationIdx++);
    if (rotationIdx > 0)
    {
        rotationIdx--;
    }

    XMVECTOR
        translate   = m_translation.size()  ? m_translation[translationIdx].value   : g_XMZero,
        scale       = m_scale.size()        ? m_scale[scaleIdx].value               : g_XMOne,
        rotation    = m_rotation.size()     ? m_rotation[rotationIdx].value         : g_XMZero;

    transform = XMMatrixAffineTransformation(scale, g_XMZero, rotation, translate);
        //XMMatrixTransformation(g_XMZero, g_XMZero, scale.value, g_XMZero, rotation.value, translate.value));
}

f32 AnimationChannel::getMaxTime()
{
    if (m_maxTime < 0.0f)
    {
        f32 maxTime = (m_translation[m_translation.size() - 1]).timeStamp;
        m_maxTime = maxTime;

        maxTime = (m_scale[m_scale.size() - 1]).timeStamp;
        if (maxTime > m_maxTime)
        {
            m_maxTime = maxTime;
        }

        maxTime = (m_rotation[m_rotation.size() - 1]).timeStamp;
        if (maxTime > m_maxTime)
        {
            m_maxTime = maxTime;
        }
    }

    return m_maxTime;
}

