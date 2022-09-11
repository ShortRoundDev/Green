#include "Animator.h"

Animator::Animator(Animation* animation) :
    m_deltaTime(0.0f),
    m_currentTime(0.0f),
    m_currentAnimation(animation)
{
    m_finalTransforms.resize(100);
}

Animator::~Animator()
{

}

void Animator::update(f32 dt)
{
    m_deltaTime = dt;

    if (m_currentAnimation && m_currentAnimation->getBoneInfoMap().size() > 0)
    {
        m_currentTime += m_currentAnimation->getTicksPerSecond() * dt;
        m_currentTime = std::fmodf(m_currentTime, m_currentAnimation->getDuration());
        calculateBoneTransform(&m_currentAnimation->getRootNode(), XMMatrixIdentity());
    }
}

void Animator::playAnimation(Animation* animation)
{
    m_currentAnimation = animation;
    m_currentTime = 0.0f;
}

void Animator::calculateBoneTransform(const AnimNodeData* node, XMMATRIX parentTransform)
{
    std::string name = node->name;
    XMMATRIX transform = XMMatrixIdentity();//node->transform;

    Bone* bone = m_currentAnimation->findBone(name);

    if (bone)
    {

        bone->update(m_currentTime);
        transform = bone->getLocalTransform();
    }

    XMMATRIX globalTransform = XMMatrixMultiply(parentTransform, transform);

    auto boneInfoMap = m_currentAnimation->getBoneInfoMap();
    auto boneInfoIter = boneInfoMap.find(name);

    if (boneInfoIter != boneInfoMap.end())
    {
        i32 index = boneInfoIter->second.id;
        XMMATRIX offset = XMMatrixIdentity();//boneInfoIter->second.offset;
        m_finalTransforms[index] = XMMatrixMultiply(globalTransform, offset);
    }

    for (int i = 0; i < node->childCount; i++)
    {
        calculateBoneTransform(&node->children[i], XMMatrixIdentity());
    }

}

const std::vector<XMMATRIX>& Animator::getFinalBoneMatrices()
{
    return m_finalTransforms;
}

void Animator::bind(const XMMATRIX& modelTransform)
{

}