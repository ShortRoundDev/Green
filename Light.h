#pragma once

#include "GTypes.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>

using namespace DirectX;
using namespace Microsoft::WRL;

class Shader;
class Scene;

struct LightSpaceBuffer
{
    XMFLOAT4 color;
    XMMATRIX lightSpace;
};

class Light
{
public:
    Light(XMFLOAT4 pos, XMFLOAT4 color, XMFLOAT4 dir, u32 width, u32 height);
    ~Light();

    void bindShadowMap();
    void renderShadowMap(Scene* scene);

private:
    u32 m_width;
    u32 m_height;

    XMFLOAT4 m_pos;
    XMFLOAT4 m_color;
    XMFLOAT4 m_dir;

    f32 m_near;
    f32 m_far;

    XMMATRIX m_projection;
    XMMATRIX m_view;
    XMMATRIX m_space;

    ComPtr<ID3D11ShaderResourceView> m_depthMapSrv;
    ComPtr<ID3D11DepthStencilView> m_depthMapDsv;

    D3D11_VIEWPORT m_viewport;

    Shader* m_shader;
};

