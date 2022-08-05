#pragma once

#include "GTypes.h"

#include "DirectXMath.h"

using namespace DirectX;

struct KeyFrame
{
public:
    KeyFrame() :
        m_timestamp(0.0f),
        m_translation(g_XMZero),
        m_scale(g_XMOne3),
        m_rotation(g_XMZero)
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

    XMFLOAT4 getRotation()
    {
        return m_rotation;
    }

private:
    f32 m_timestamp;
    XMFLOAT3 m_translation;
    XMFLOAT3 m_scale;
    XMFLOAT4 m_rotation;
};
