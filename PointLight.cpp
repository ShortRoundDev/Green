#include "PointLight.h"

#include "GraphicsManager.h"
#include "Scene.h"
#include "Shader.h"
#include "Sprite.h"

#include "MapFile_Parse.h"

PointLight* PointLight::Create(MF_Entity* entity)
{
    MF_Vector3 pos;
    MF_Vector4 color;
    f32 radius;
    f32 cutoff;

    if (!MF_GetAttributeVec3(entity, "origin", &pos))
    {
        return nullptr;
    }

    if (!MF_GetAttributeVec4(entity, "color", &color))
    {
        return nullptr;
    }

    if (!MF_GetAttributeFloat(entity, "radius", &radius))
    {
        return nullptr;
    }

    if (!MF_GetAttributeFloat(entity, "cutoff", &cutoff))
    {
        return nullptr;
    }

    return new PointLight(
        MF3_TO_XM4(pos),
        MFCOL_TO_XM4(color),
        SHADOW_RES, SHADOW_RES,
        radius, cutoff
    );
}

PointLight::PointLight(
    XMFLOAT4 pos,
    XMFLOAT4 color,
    u32 width,
    u32 height,
    f32 radius,
    f32 cutoff
) :
    ILight(
        pos,
        color,
        width,
        height,
        POINT_LIGHT
    ),
    m_radius(radius),
    m_cutoff(cutoff)
{
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 6;
    desc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

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
    dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
    dsv.Texture2DArray.MipSlice = 0;
    dsv.Texture2DArray.ArraySize = 1;
    dsv.Texture2DArray.FirstArraySlice = 0;
    
    for (int i = 0; i < 6; i++)
    {
        dsv.Texture2DArray.FirstArraySlice = D3D11CalcSubresource(0, i, 1);
        Graphics.getDevice()->CreateDepthStencilView(depthMap, &dsv, m_shadowMapFacesDsv[i].GetAddressOf());
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC srv;
    srv.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srv.TextureCube.MipLevels = desc.MipLevels;
    srv.TextureCube.MostDetailedMip = 0;
    auto result = Graphics.getDevice()->CreateShaderResourceView(depthMap, &srv, m_depthMapSrv.GetAddressOf());
    m_projection = XMMatrixPerspectiveFovLH(M_PI_2_F, (f32)width / (f32)height, 0.1f, 1000.0f);

    m_shader = Graphics.getShader(L"SpotLight");

    //calculate cbuffer
    XMVECTOR eyePos = XMLoadFloat4(&m_pos);
    XMFLOAT4 upFloat4 = { 0, 1.0f, 0, 1.0f };
    XMVECTOR up = XMLoadFloat4(&upFloat4);

    for (int i = 0; i < 6; i++)
    {
        XMFLOAT4 upFloat4 = { 0, 1.0f, 0, 1.0f };
        XMVECTOR up = XMLoadFloat4(&upFloat4);

        bindShadowMap(i);
        XMFLOAT4 lookAheadFloat4;// = { 0, 0, 1, 1 };
        switch (i)
        {
        case 0:
            lookAheadFloat4 = { 1, 0, 0, 1 };
            break;
        case 1:
            lookAheadFloat4 = { -1, 0, 0, 1 };
            break;
        case 2:
            lookAheadFloat4 = { 0, 1, 0, 1.0f };
            upFloat4 = { 0, 0, -1, 1 };
            up = XMLoadFloat4(&upFloat4);
            break;
        case 3:
            lookAheadFloat4 = { 0, -1, 0, 1 };
            upFloat4 = { 0, 0, 1, 1 };
            up = XMLoadFloat4(&upFloat4);
            break;
        case 4:
            lookAheadFloat4 = { 0, 0, 1, 1 };
            break;
        case 5:
            lookAheadFloat4 = { 0, 0, -1, 1 };
            break;
        }

        XMVECTOR dir = XMLoadFloat4(&lookAheadFloat4);
        XMVECTOR lookAtPos = XMVectorAdd(eyePos, dir);
        m_view = XMMatrixLookAtLH(eyePos, lookAtPos, up);
        m_space = XMMatrixMultiply(m_view, m_projection);
        XMMATRIX m_spaceT = XMMatrixTranspose(m_space);

        m_cBuffer.lightPos = m_pos;
        m_cBuffer.color = m_color;
        m_cBuffer.lightSpace[i] = m_spaceT;
        m_cBuffer.radius = radius;
        m_cBuffer.cutoff = cutoff;
    }

    m_sprite = new Sprite("textures\\sprites\\001-lightbulb.png", { m_pos.x, m_pos.y, m_pos.z }, m_color);
}

PointLight::~PointLight()
{

}

void PointLight::use(u32 slot)
{
    Graphics.getContext()->PSSetShaderResources(slot, 1, m_depthMapSrv.GetAddressOf());
}

void PointLight::renderShadowMap(Scene* scene)
{
    Graphics.setShadowRasterizer(true);
    m_shader->use();

    XMVECTOR eyePos = XMLoadFloat4(&m_pos);
    XMFLOAT4 upFloat4 = { 0, 1.0f, 0, 1.0f };
    XMVECTOR up = XMLoadFloat4(&upFloat4);
    bindShadowMap(0);

    for (int i = 0; i < 6; i++)
    {
        XMFLOAT4 upFloat4 = { 0, 1.0f, 0, 1.0f };
        XMVECTOR up = XMLoadFloat4(&upFloat4);

        bindShadowMap(i);
        XMFLOAT4 lookAheadFloat4;// = { 0, 0, 1, 1 };
        switch (i)
        {
        case 0:
            lookAheadFloat4 = { 1, 0, 0, 1 };
            break;
        case 1:
            lookAheadFloat4 = { -1, 0, 0, 1 };
            break;
        case 2:
            lookAheadFloat4 = { 0, 1, 0, 1.0f };
            upFloat4 = { 0, 0, -1, 1 };
            up = XMLoadFloat4(&upFloat4);
            break;
        case 3:
            lookAheadFloat4 = { 0, -1, 0, 1 };
            upFloat4 = { 0, 0, 1, 1 };
            up = XMLoadFloat4(&upFloat4);
            break;
        case 4:
            lookAheadFloat4 = { 0, 0, 1, 1 };
            break;
        case 5:
            lookAheadFloat4 = { 0, 0, -1, 1 };
            break;
        }

        XMVECTOR dir = XMLoadFloat4(&lookAheadFloat4);
        XMVECTOR lookAtPos = XMVectorAdd(eyePos, dir);
        m_view = XMMatrixLookAtLH(eyePos, lookAtPos, up);
        m_space = XMMatrixMultiply(m_view, m_projection);
        XMMATRIX m_spaceT = XMMatrixTranspose(m_space);

        LightSpaceBuffer buffer = {
            m_spaceT,
            m_color,
            m_pos
        };

        m_shader->bindCBuffer(&buffer);
        m_shader->bindModelMatrix(XMMatrixIdentity());
        scene->renderMeshes();
    }
    Graphics.resetRenderTarget();
    Graphics.resetViewport();
    Graphics.setShadowRasterizer(false);

}

void PointLight::bindShadowMap(u32 face)
{
    auto context = Graphics.getContext();
    context->RSSetViewports(1, &m_viewport);
    ID3D11RenderTargetView* renderTargets[1] = { 0 };
    context->OMSetRenderTargets(1, renderTargets, m_shadowMapFacesDsv[face].Get());
    context->ClearDepthStencilView(m_shadowMapFacesDsv[face].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

PointLightBuffer PointLight::getCBuffer()
{
    return m_cBuffer;
}

f32 PointLight::maxDistance()
{
    if (m_cutoff == 0.0f)
    {
        return 0.0f;
    }
    return m_radius / sqrtf(m_cutoff);
}

AABB PointLight::getBounds()
{
    f32 dist = maxDistance();
    XMFLOAT3 min = XMFLOAT3(m_pos.x - dist, m_pos.y - dist, m_pos.z - dist);
    XMFLOAT3 max = XMFLOAT3(m_pos.x + dist, m_pos.y + dist, m_pos.z + dist);
    return AABB(min, max);
}

void PointLight::draw()
{
    if (m_sprite)
    {
        m_sprite->draw();
    }
}