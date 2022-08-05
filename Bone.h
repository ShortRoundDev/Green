#pragma once

#include "GTypes.h"
#include "KeyFrame.h"

#include <DirectXMath.h>

#include <vector>

using namespace DirectX;

class Bone
{
public:
    Bone()
    {

    }
    ~Bone()
    {

    }

    f32 getStartTime()
    {

    }

    f32 getEndTime()
    {

    }

    void interpolate(f32 t, XMMATRIX& transform)
    {
        if (t <= keyFrames.front().getTimestamp())
        {
            auto front = keyFrames.front();
            auto scale = front.getScale();
            auto translation = front.getTranslation();
            auto rotation = front.getRotation();
            XMVECTOR s = XMLoadFloat3(&scale);
            XMVECTOR p = XMLoadFloat3(&translation);
            XMVECTOR q = XMLoadFloat4(&rotation);
            
            XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
            transform = XMMatrixAffineTransformation(s, zero, q, p);
        }
        else if (t >= keyFrames.back().getTimestamp())
        {
            auto back = keyFrames.back();
            auto scale = back.getScale();
            auto translation = back.getTranslation();
            auto rotation = back.getRotation();
            XMVECTOR s = XMLoadFloat3(&scale);
            XMVECTOR p = XMLoadFloat3(&translation);
            XMVECTOR q = XMLoadFloat4(&rotation);

            XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
            transform = XMMatrixAffineTransformation(s, zero, q, p);
        }
        else
        {
            for (u32 i = 0; i < keyFrames.size() - 1; i++)
            {
                auto j = keyFrames[i]; // j = last frame completed
                auto k = keyFrames[i + 1]; // k = next frame, to be completed
                if (t >= j.getTimestamp() && t <= k.getTimestamp())
                {
                    float lerpPercent = (t - j.getTimestamp()) / (k.getTimestamp() - j.getTimestamp());

                    auto scale0 = j.getScale();
                    auto scale1 = k.getScale();
                    XMVECTOR s0 = XMLoadFloat3(&scale0);
                    XMVECTOR s1 = XMLoadFloat3(&scale1);

                    auto translation0 = j.getTranslation();
                    auto translation1 = k.getTranslation();
                    XMVECTOR p0 = XMLoadFloat3(&translation0);
                    XMVECTOR p1= XMLoadFloat3(&translation1);

                    auto rotation0 = j.getRotation();
                    auto rotation1 = k.getRotation();
                    XMVECTOR q0 = XMLoadFloat4(&rotation0);
                    XMVECTOR q1 = XMLoadFloat4(&rotation1);

                    XMVECTOR s = XMVectorLerp(s0, p1, lerpPercent);
                    XMVECTOR p = XMVectorLerp(p0, p1, lerpPercent);
                    XMVECTOR q = XMQuaternionSlerp(q0, q1, lerpPercent);

                    XMVECTOR zero = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
                    transform = XMMatrixAffineTransformation(s, zero, q, p);

                    break;
                }
            }
        }
    }
    std::vector<KeyFrame> keyFrames;
};

