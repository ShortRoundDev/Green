#include "AnimationJoint.h"

AnimationJoint::AnimationJoint(u32 boneId, XMMATRIX inverseBindMatrix, AnimationChannel animationChannel, XMMATRIX localMatrix, XMFLOAT3 pos) :
    m_inverseBindMatrix(inverseBindMatrix),
    m_boneId(boneId),
    m_animationChannel(animationChannel),
    m_localMatrix(localMatrix),
    m_pos(pos)
{
    
}

AnimationJoint::~AnimationJoint()
{

}

XMMATRIX AnimationJoint::getTransformation(f32 time)
{
    XMMATRIX boneTransform;
    m_animationChannel.getBoneTransform(boneTransform, time);
    return boneTransform;
}

XMMATRIX AnimationJoint::getInverseBind()
{
    return m_inverseBindMatrix;
}

XMMATRIX AnimationJoint::getLocalMatrix()
{
    return m_localMatrix;
}

XMMATRIX AnimationJoint::getSkinMatrix(XMMATRIX parentSkinMatrix)
{
    XMMATRIX boneTransform;
    m_animationChannel.getBoneTransform(boneTransform, 0.0f);
    return XMMatrixMultiply(parentSkinMatrix, XMMatrixMultiply(m_inverseBindMatrix, boneTransform));
}

f32 AnimationJoint::getMaxTime()
{
    if (m_maxTime < 0.0f)
    {
        m_maxTime = m_animationChannel.getMaxTime();
    }
    return m_maxTime;
}

const XMFLOAT3& AnimationJoint::getPos()
{
    return m_pos;
}