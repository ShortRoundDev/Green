#pragma once

#include "GTypes.h"

#include "DirectXMath.h"

using namespace DirectX;

struct KeyFrame
{
public:
    KeyFrame()
    {

    }

    ~KeyFrame()
    {

    }

    f32 getTimestamp()
    {
        return m_timestamp;
    }
    
    XMFLOAT3 getTranslation()
    {
        return m_translation;
    }
    
    XMFLOAT3 getScale()
    {
        return m_scale;
    }

    XMFLOAT3 getRotation()
    {
        return m_rotation;
    }

private:
    f32 m_timestamp;
    XMFLOAT3 m_translation;
    XMFLOAT3 m_scale;
    XMFLOAT3 m_rotation;
};
