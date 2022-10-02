#include "AnimationSkeleton.h"

AnimationSkeleton::AnimationSkeleton() :
    m_boneHierarchy(nullptr),
    m_joints()
{

}

AnimationSkeleton::AnimationSkeleton(u8* boneHierarchy, std::vector<AnimationJoint> joints) :
    m_boneHierarchy(boneHierarchy),
    m_joints(joints)
{

}

AnimationSkeleton::~AnimationSkeleton()
{
    //delete m_boneHierarchy;
}

void AnimationSkeleton::getFinalMatrix(std::vector<XMMATRIX>& finalMatrix)
{
    finalMatrix.reserve(m_joints.size());
    std::vector<XMMATRIX> toParent(m_joints.size());
    for (u32 i = 0; i < (u32)m_joints.size(); i++)
    {
        toParent[i] = m_joints[i].getTransformation();
    }

    std::vector<XMMATRIX> toRoot(m_joints.size());
    toRoot[0] = toParent[0];

    for (u32 i = 1; i < (u32)m_joints.size(); i++)
    {
        XMMATRIX toParentTransform = toParent[i];
        int parentIndex = m_boneHierarchy[i];

        XMMATRIX parentToRoot = toRoot[parentIndex];
        XMMATRIX toRootTransform = XMMatrixMultiply(toParentTransform, parentToRoot);

        toRoot[i] = toRootTransform;
    }

    for (u32 i = 0; i < m_joints.size(); i++)
    {
        XMMATRIX offset = m_joints[i].getInverseBind();
        XMMATRIX toRootTransform = toRoot[i];
        finalMatrix.push_back(XMMatrixMultiply(offset, toRootTransform));
    }
}