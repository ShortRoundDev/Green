#pragma once

#include "GTypes.h"
#include "Bone.h"
#include "BoneInfo.h"

#include "Mesh.h"

#include <string>
#include <DirectXMath.h>

#include <assimp/anim.h>
#include <assimp/scene.h>

#include <map>
#include <string>
#include <vector>

using namespace DirectX;

struct AnimNodeData
{
    XMMATRIX transform;
    std::string name;
    i32 childCount;
    std::vector<AnimNodeData> children;
};

class Animation
{
public:
    Animation();
    ~Animation();

    Animation(std::string animationPath, Mesh* mesh);

    Bone* findBone(const std::string& name);

    i32 getTicksPerSecond();
    f32 getDuration();
    const AnimNodeData& getRootNode();
    const std::map<std::string, BoneInfo>& getBoneInfoMap();

private:
    f32 m_duration;
    i32 m_ticksPerSecond;
    std::vector<Bone> m_bones;
    AnimNodeData m_rootNode;
    std::map<std::string, BoneInfo> m_boneInfoMap;

    void readHierarchyData(AnimNodeData& node, const aiNode* src);
    void readMissingBones(const aiAnimation* animation, Mesh* mesh);
};

