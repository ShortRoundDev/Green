#pragma once

#include "AnimationClip.h"

#include "DirectXMath.h"

#include <vector>
#include <map>
#include <string>

class SkinnedData
{
public:
    SkinnedData(
        std::vector<i32>& boneHierarchy,
        std::vector<XMMATRIX>& boneOffsets,
        std::map<std::string, AnimationClip>& animations
    ) :
        m_boneHierarchy(boneHierarchy),
        m_boneOffsets(boneOffsets),
        m_animations(animations)
    {
        
    }

    sz getBoneCount()
    {
        return m_boneOffsets.size();
    }
    void getFinalTransforms(const std::string& name, f32 timePos, std::vector<XMMATRIX>& finalTransforms)
    {
        u32 numBones = m_boneOffsets.size();
        std::vector<XMMATRIX> toParentTransforms(numBones);

        auto clip = m_animations.find(name);
        clip->second.interpolate(timePos, toParentTransforms);

        std::vector<XMMATRIX> toRootTransforms(numBones);

        toRootTransforms[0] = toParentTransforms[0];

        for (u32 i = 1; i < numBones; i++)
        {
            auto toParent = toParentTransforms[i];

            i32 parentIndex = m_boneHierarchy[i];
            auto parentToRoot = toRootTransforms[parentIndex];
            
            auto toRoot = XMMatrixMultiply(toParent, parentToRoot);
            
            toRootTransforms[i] = toRoot;
        }

        for (u32 i = 0; i < numBones; i++)
        {
            auto offset = m_boneOffsets[i];
            auto toRoot = toRootTransforms[i];

            finalTransforms[i] = XMMatrixMultiply(offset, toRoot);
        }
    }

private:
    std::vector<i32> m_boneHierarchy;
    std::vector<XMMATRIX> m_boneOffsets;
    std::map<std::string, AnimationClip> m_animations;
};
