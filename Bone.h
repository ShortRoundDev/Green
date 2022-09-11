#pragma once

#include "DirectXMath.h"
#include "assimp/anim.h"

#include "GTypes.h"

#include <vector>
#include <string>
#include <functional>

using namespace DirectX;

typedef XMVECTOR(*vectorInterpolator)(XMVECTOR a0, XMVECTOR a1, f32 t);

struct KeyFrame
{
    XMVECTOR value;
    f32 timestamp;
};

class Bone
{
public:
    Bone(const std::string& name, i32 id, const aiNodeAnim* channel);
    ~Bone();
    void update(f32 time);
    const XMMATRIX& getLocalTransform();

private:
    std::vector<KeyFrame> m_positions;
    std::vector<KeyFrame> m_scales;
    std::vector<KeyFrame> m_rotations;

    i32 m_numPositions;
    i32 m_numRotations;
    i32 m_numScales;

    XMMATRIX m_localTransform;
    std::string m_name;
    i32 m_id;

    i32 getKeyframeIndex(const std::vector<KeyFrame>& keyframes, f32 time);

    void getKeyFrames(const std::vector<KeyFrame>& keyframes, f32 time, XMVECTOR& a0, XMVECTOR& a1, f32& timePct);

    template<typename T>
    void loadKeyFrames(std::vector<KeyFrame>& keyframes, T* channel, i32 keyLength, std::function<KeyFrame(T)> transformer)
    {
        for (int i = 0; i < keyLength; i++)
        {
            T aiKey = channel[i];

            KeyFrame animKey = transformer(aiKey);

            keyframes.push_back(animKey);
        }
    }
};
