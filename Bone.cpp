#include "Bone.h"

#include "assimp/anim.h"

#include <algorithm>

Bone::Bone(const std::string& name, i32 id, const aiNodeAnim* channel) :
    m_name(name),
    m_id(id),
    m_localTransform(XMMatrixIdentity()),
    m_numPositions(channel->mNumPositionKeys),
    m_numRotations(channel->mNumRotationKeys),
    m_numScales(channel->mNumScalingKeys)
{
    m_positions.reserve(channel->mNumPositionKeys);
    m_rotations.reserve(channel->mNumRotationKeys);
    m_scales.reserve(channel->mNumScalingKeys);

    loadKeyFrames<aiVectorKey>(m_positions, channel->mPositionKeys, m_numPositions, [](aiVectorKey vec) -> KeyFrame {
        return {
            XMVectorSet(vec.mValue.x, vec.mValue.y, vec.mValue.z, 0.0f),
            (f32)vec.mTime
        };
    });

    loadKeyFrames<aiVectorKey>(m_scales, channel->mScalingKeys, m_numScales, [](aiVectorKey vec) -> KeyFrame {
        return {
            XMVectorSet(vec.mValue.x, vec.mValue.y, vec.mValue.z, 0.0f),
            (f32)vec.mTime
        };
    });

    loadKeyFrames<aiQuatKey>(m_rotations, channel->mRotationKeys, m_numRotations, [](aiQuatKey quat) -> KeyFrame {
        return {
            XMVectorSet(quat.mValue.x, quat.mValue.y, quat.mValue.z, quat.mValue.w),
            (f32)quat.mTime
        };
    });
}

Bone::~Bone()
{

}

void Bone::update(f32 time)
{
    static const XMVECTOR ZERO = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

    //XMMATRIX translation = XM
    XMVECTOR
        t0, t1,
        s0, s1,
        r0, r1;

    f32 tPct, sPct, rPct;

    getKeyFrames(m_positions, time, t0, t1, tPct);
    getKeyFrames(m_rotations, time, r0, r1, rPct);
    getKeyFrames(m_scales, time, s0, s1, sPct);

    XMVECTOR
        t = XMVectorLerp(t0, t1, tPct),
        r = XMQuaternionSlerp(r0, r1, rPct),
        s = XMVectorLerp(s0, s1, sPct);

    m_localTransform = XMMatrixAffineTransformation(s, ZERO, r, t);
}

const XMMATRIX& Bone::getLocalTransform()
{
    return m_localTransform;
}

i32 Bone::getKeyframeIndex(const std::vector<KeyFrame>& keyframes, f32 time)
{
    for (int i = 0; i < keyframes.size() - 1; i++)
    {
        if (time < keyframes[i + 1].timestamp)
        {
            return i;
        }
    }
    return -1;
}

void Bone::getKeyFrames(const std::vector<KeyFrame>& keyframes, f32 time, XMVECTOR& a0, XMVECTOR& a1, f32& timePct)
{
    auto idx0 = getKeyframeIndex(keyframes, time);
    auto idx1 = idx0 + 1;

    a0 = keyframes[idx0].value;
    a1 = keyframes[idx1].value;

    timePct = (time - keyframes[idx0].timestamp) / (keyframes[idx1].timestamp - keyframes[idx0].timestamp);
}