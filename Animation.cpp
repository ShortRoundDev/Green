#include "Animation.h"

#include "assimp/Importer.hpp"
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include "Mesh.h"
#include "Util.h"

Animation::Animation()
{

}

Animation::~Animation()
{

}

Animation::Animation(std::string animationPath, Mesh* mesh)
{
    Assimp::Importer importer;
    auto scene = importer.ReadFile(animationPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
    if (scene->mNumAnimations == 0)
    {
        return;
    }
    auto animation = scene->mAnimations[0];
    m_duration = (f32)animation->mDuration;
    m_ticksPerSecond = (i32)animation->mTicksPerSecond;

    readHierarchyData(m_rootNode, scene->mRootNode);
    readMissingBones(animation, mesh);
}

Bone* Animation::findBone(const std::string& name)
{
    auto res = m_boneInfoMap.find(name);
    if (res == m_boneInfoMap.end())
    {
        return nullptr;
    }
    return m_bones.data() + res->second.id; // woop woop
}

f32 Animation::getTicksPerSecond()
{
    return m_ticksPerSecond;
}

f32 Animation::getDuration()
{
    return m_duration;
}

const AnimNodeData& Animation::getRootNode()
{
    return m_rootNode;
}

const std::map<std::string, BoneInfo>& Animation::getBoneInfoMap()
{
    return m_boneInfoMap;
}

void Animation::readHierarchyData(AnimNodeData& node, const aiNode* src)
{
    node.name = src->mName.data;
    convertAiMatrixToXMMatrix(src->mTransformation, node.transform);
    node.childCount = src->mNumChildren;

    for (int i = 0; i < src->mNumChildren; i++)
    {
        AnimNodeData newData;
        readHierarchyData(newData, src->mChildren[i]);
        node.children.push_back(newData);
    }
}

void Animation::readMissingBones(const aiAnimation* animation, Mesh* mesh)
{
    u32 size = animation->mNumChannels;
    auto& boneInfoMap = mesh->getBoneInfoMap();

    i32 boneCount = mesh->getBoneCounter();

    for (int i = 0; i < size; i++)
    {
        auto channel = animation->mChannels[i];
        std::string boneName = channel->mNodeName.data;

        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            boneInfoMap[boneName].id = boneCount;
            boneCount++;
            mesh->setBoneCounter(boneCount);
        }
        m_bones.push_back(Bone(boneName, boneInfoMap[boneName].id, channel));
    }

    m_boneInfoMap = boneInfoMap;
}