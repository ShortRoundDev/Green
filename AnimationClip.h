#pragma once

#include "GTypes.h"
#include "Bone.h"

#include "DirectXMath.h"

#include <vector>

using namespace DirectX;

class AnimationClip
{
public:
    AnimationClip(const std::vector<Bone>& bones) :
        m_bones(bones),
        m_startTime(0.0f),
        m_endTime(0.0f)
    {
        f32 startTime = FLT_MAX;
        f32 endTime = -1.0f;
        for (auto bone : m_bones)
        {
            if (bone.getStartTime() < startTime)
            {
                startTime = bone.getStartTime();
            }
            if (bone.getEndTime() > endTime)
            {
                endTime = bone.getEndTime();
            }
        }
    }
    ~AnimationClip()
    {

    }

    f32 getStartTime()
    {
        return m_startTime;
    }
    f32 getEndTime()
    {
        return m_endTime;
    }

    void interpolate(f32 t, std::vector<XMMATRIX>& boneTransforms)
    {
        for (auto bone : m_bones)
        {
            XMMATRIX transform;
            bone.interpolate(t, transform);
            boneTransforms.push_back(transform);
        }
    }

private:
    std::vector<Bone> m_bones;
    f32 m_startTime;
    f32 m_endTime;
};

