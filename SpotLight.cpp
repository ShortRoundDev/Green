#include "SpotLight.h"

#include "GraphicsManager.h"
#include "SystemManager.h"
#include "Shader.h"
#include "Scene.h"

SpotLight::SpotLight(
    XMFLOAT4 pos,
    XMFLOAT4 color,
    u32 width,
    u32 height,
    XMFLOAT4 dir
) :
    ILight(pos, color, width, height),
    m_dir(dir)
{
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
    m_projection = XMMatrixPerspectiveFovLH(M_PI_4_F, (f32)width/(f32)height, 0.1f, 1000.0f);
    //m_projection = XMMatrixOrthographicLH(width, height, m_near, m_far);

    XMFLOAT4 upFloat4 = { 0, 1.0f, 0, 1.0f };

    XMVECTOR eyePos = XMLoadFloat4(&pos);
    XMVECTOR dirVec = XMLoadFloat4(&dir);
    XMVECTOR lookPos = XMVectorAdd(eyePos, dirVec);
    XMVECTOR up = XMLoadFloat4(&upFloat4);

    m_view = XMMatrixLookAtLH(eyePos, lookPos, up);
    m_space = XMMatrixMultiply(m_view, m_projection);

    m_shader = Graphics.getShader(L"SpotLight");
}

SpotLight::~SpotLight()
{

}

void SpotLight::bindShadowMap()
{
    auto context = Graphics.getContext();
    context->RSSetViewports(1, &m_viewport);
    ID3D11RenderTargetView* renderTargets[1] = { 0 };
    context->OMSetRenderTargets(1, renderTargets, m_depthMapDsv.Get());
    context->ClearDepthStencilView(m_depthMapDsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void SpotLight::use(u32 slot)
{
    Graphics.getContext()->PSSetShaderResources(slot, 1, m_depthMapSrv.GetAddressOf());
}

void SpotLight::renderShadowMap(Scene* scene)
{
    Graphics.setShadowRasterizer(true);
    bindShadowMap();
    m_shader->use();

    XMMATRIX m_spaceT = XMMatrixTranspose(m_space);

    LightSpaceBuffer buffer = {
        m_spaceT,
        m_color,
        m_pos
    };
    
    m_shader->bindCBuffer(&buffer);

    scene->renderMeshes();
    Graphics.resetRenderTarget();
    Graphics.resetViewport();
    Graphics.setShadowRasterizer(false);
}
