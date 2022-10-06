#include "AnimationSkeleton.h"

#include "Util.h"

#include "Logger.h"
static Logger logger = ::CreateLogger("AnimationSkeleton");

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

void AnimationSkeleton::getFinalMatrix(std::vector<XMMATRIX>& finalMatrix, std::vector<XMMATRIX>& inverseBindMatrices)
{
    finalMatrix.reserve(m_joints.size());
    std::vector<XMMATRIX> toParentTransforms(m_joints.size());

    for (u32 i = 0; i < (u32)m_joints.size(); i++)
    {
        // get each bone's animation
        // at the current frame
        toParentTransforms[i] = m_joints[i].getTransformation();
    }

    std::vector<XMMATRIX> toRootTransforms(m_joints.size());
    toRootTransforms[0] = toParentTransforms[0];

    for (u32 i = 1; i < (u32)m_joints.size(); i++)
    {
        XMMATRIX toParent = toParentTransforms[i];
        
        // m_boneHierarchy is an array of integers.
        // index = bone ID, value = parent bone index
        int parentIndex = m_boneHierarchy[i];

        // all ancestor's animation transforms multiplied together
        XMMATRIX parentToRoot = toRootTransforms[parentIndex];

        XMMATRIX toRoot = XMMatrixMultiply(
            toParent, parentToRoot
        );

        toRootTransforms[i] = toRoot;
    }

    for (u32 i = 0; i < m_joints.size(); i++)
    {
        XMMATRIX ibm = inverseBindMatrices[i];
        XMMATRIX otherIbm = m_joints[i].getInverseBind();
        
        logger.info("Calculated IBM:");
        printMatrix(&logger, ibm);
        logger.info("Deserialized IBM:");
        printMatrix(&logger, otherIbm);

        XMMATRIX toRoot = toRootTransforms[i];
        // multiply toRoot by inverse Bind Matrix
        auto final = XMMatrixMultiply(ibm, toRoot);

        finalMatrix.push_back(final);
    }
}

std::vector<AnimationJoint>& AnimationSkeleton::getJoints()
{
    return m_joints;
}

u8* AnimationSkeleton::getBoneHierarchy()
{
    return m_boneHierarchy;
}