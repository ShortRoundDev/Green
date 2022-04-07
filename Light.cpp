#include "Light.h"

#include "GraphicsManager.h"
#include "SystemManager.h"
#include "Shader.h"
#include "Scene.h"

Light::Light(
    XMFLOAT4 pos,
    XMFLOAT4 color,
    XMFLOAT4 dir,
    u32 width,
    u32 height
) :
    m_width(width),
    m_height(height),
    m_pos(pos),
    m_color(color),
    m_dir(dir),
    m_near(0.1f),
    m_far(1000.0f)
{
    m_viewport.TopLeftX = 0.0f;
    m_viewport.TopLeftY = 0.0f;
    m_viewport.Width = (float)width;
    m_viewport.Height = (float)height;
    m_viewport.MinDepth = 0.0f;
    m_viewport.MaxDepth = 1.0f;

    D3D11_TEXTURE2D_DESC desc;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    ID3D11Texture2D* depthMap = NULL;
    Graphics.getDevice()->CreateTexture2D(&desc, 0, &depthMap);
    if (depthMap == NULL)
    {
        //err
        return;
    }
    D3D11_DEPTH_STENCIL_VIEW_DESC dsv;
    dsv.Flags = 0;
    dsv.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsv.Texture2D.MipSlice = 0;
    Graphics.getDevice()->CreateDepthStencilView(depthMap, &dsv, m_depthMapDsv.GetAddressOf());

    D3D11_SHADER_RESOURCE_VIEW_DESC srv;
    srv.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv.Texture2D.MipLevels = desc.MipLevels;
    srv.Texture2D.MostDetailedMip = 0;
    Graphics.getDevice()->CreateShaderResourceView(depthMap, &srv, m_depthMapSrv.GetAddressOf());

    ///// Matrices /////
    auto vars = System.getVars();
    //m_projection = XMMatrixPerspectiveFovLH(M_PI_4_F, (f32)width/(f32)height, 0.1f, 1000.0f);
    m_projection = XMMatrixOrthographicLH(width, height, m_near, m_far);

    XMFLOAT4 upFloat4 = { 0, 1.0f, 0, 1.0f };

    XMVECTOR eyePos = XMLoadFloat4(&pos);
    XMVECTOR dirVec = XMLoadFloat4(&dir);
    XMVECTOR lookPos = XMVectorAdd(eyePos, dirVec);
    XMVECTOR up = XMLoadFloat4(&upFloat4);

    m_view = XMMatrixLookAtLH(eyePos, lookPos, up);
    m_space = XMMatrixMultiply(m_view, m_projection);

    m_shader = Graphics.getShader(L"Light");
}

Light::~Light()
{

}

void Light::bindShadowMap()
{
    auto context = Graphics.getContext();
    context->RSSetViewports(1, &m_viewport);
    ID3D11RenderTargetView* renderTargets[1] = { 0 };
    context->OMSetRenderTargets(1, renderTargets, m_depthMapDsv.Get());
    context->ClearDepthStencilView(m_depthMapDsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}
void Light::renderShadowMap(Scene* scene)
{
    bindShadowMap();
    m_shader->use();

    XMMATRIX m_spaceT = XMMatrixTranspose(m_space);

    LightSpaceBuffer buffer = {
        { 1.0f, 2.0f, 3.0f, 4.0f },
        m_spaceT
    };
    
    m_shader->bindCBuffer(&buffer);

    scene->renderMeshes();
    Graphics.resetRenderTarget();
    Graphics.resetViewport();
}
