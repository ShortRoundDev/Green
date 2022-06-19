#pragma once

#include "GTypes.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

#include "AABB.h"

using namespace DirectX;
using namespace Microsoft::WRL;

constexpr u32 MAX_LIGHT = 16;

struct LightSpaceBuffer
{
    XMMATRIX lightSpace;
    XMFLOAT4 color;
    XMFLOAT4 lightPos;
};

class Shader;
class Scene;

class ILight
{
public:
    ILight(XMFLOAT4 pos, XMFLOAT4 color, u32 width, u32 height, u32 lightType) :
        m_pos(pos),
        m_color(color),
        m_width(width),
        m_height(height),
        m_far(1000.0f),
        m_near(0.01f),
        m_lightType(lightType)
    {
        m_viewport.TopLeftX = 0.0f;
        m_viewport.TopLeftY = 0.0f;
        m_viewport.Width = (float)width;
        m_viewport.Height = (float)height;
        m_viewport.MinDepth = 0.0f;
        m_viewport.MaxDepth = 1.0f;
    };
    ~ILight()
    {

    };

    virtual void renderShadowMap(Scene* scene) = 0;
    virtual void use(u32 slot) = 0;
    virtual void draw() = 0;

    XMFLOAT4 getPos();
    ID3D11ShaderResourceView* getDepthMapSrv();

    u32 getLightType();

    virtual AABB getBounds() = 0;

protected:
    u32 m_lightType;

    u32 m_width;
    u32 m_height;

    XMFLOAT4 m_pos;
    XMFLOAT4 m_color;

    f32 m_near;
    f32 m_far;

    XMMATRIX m_projection;
    XMMATRIX m_view;
    XMMATRIX m_space;

    Shader* m_shader;

    ComPtr<ID3D11ShaderResourceView> m_depthMapSrv;
    ComPtr<ID3D11DepthStencilView> m_depthMapDsv;

    ComPtr<ID3D11ShaderResourceView> m_blurredDepthMapSrv;
    ComPtr<ID3D11RenderTargetView> m_blurredRtv;

    D3D11_VIEWPORT m_viewport;
};

