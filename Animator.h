#pragma once

#include "DirectXMath.h"
#include "Animation.h"

#include <vector>

using namespace DirectX;

class Animator
{
public:
    Animator(Animation* animation);
    ~Animator();

    void update(f32 dt);
    void playAnimation(Animation* animation);
    void calculateBoneTransform(const AnimNodeData* node, XMMATRIX parentTransform);

    const std::vector<XMMATRIX>& getFinalBoneMatrices();

    void bind(const XMMATRIX& modelTransform);

private:
    std::vector<XMMATRIX> m_finalTransforms;
    Animation* m_currentAnimation;
    f32 m_currentTime;
    f32 m_deltaTime;
};

