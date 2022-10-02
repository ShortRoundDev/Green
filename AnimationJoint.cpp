#include "AnimationJoint.h"

AnimationJoint::AnimationJoint(u32 boneId, XMMATRIX inverseBindMatrix, AnimationChannel animationChannel) :
    m_inverseBindMatrix(inverseBindMatrix),
    m_boneId(boneId),
    m_animationChannel(animationChannel)
{
    
}

AnimationJoint::~AnimationJoint()
{

}

XMMATRIX AnimationJoint::getTransformation()
{
    XMMATRIX boneTransform;
    m_animationChannel.getBoneTransform(boneTransform, 0.0f);
    return boneTransform;
}

XMMATRIX AnimationJoint::getInverseBind()
{
    return m_inverseBindMatrix;
}

XMMATRIX AnimationJoint::getSkinMatrix(XMMATRIX parentSkinMatrix)
{
    XMMATRIX boneTransform;
    m_animationChannel.getBoneTransform(boneTransform, 0.0f);
    return XMMatrixMultiply(parentSkinMatrix, XMMatrixMultiply(m_inverseBindMatrix, boneTransform));
}