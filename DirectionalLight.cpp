#include "DirectionalLight.h"

#include "GraphicsManager.h"
#include "GameManager.h"
#include "Scene.h"
#include "Shader.h"
#include "Sprite.h"

#include "MapFile_Parse.h"
#include "Logger.h"

static Logger logger = CreateLogger("DirectionalLight");

DirectionalLight* DirectionalLight::Create(MF_Entity* entity)
{
    MF_Vector3 pos;
    MF_Vector4 color;
    MF_Vector3 dir;

    if (!MF_GetAttributeVec3(entity, "origin", &pos))
    {
        return nullptr;
    }

    if (!MF_GetAttributeVec4(entity, "color", &color))
    {
        return nullptr;
    }

    if (!MF_GetAttributeVec3(entity, "dir", &dir))
    {
        return nullptr;
    }

    return new DirectionalLight(
        MF3_TO_XM4(pos),
        MFCOL_TO_XM4(color),
        MF3_TO_XM4(dir),
        4096, 4096
    );
}

DirectionalLight::DirectionalLight(
    XMFLOAT4 pos,
    XMFLOAT4 color,
    XMFLOAT4 dir,
    u32 width,
    u32 height
) :
    ILight(pos, color, width, height, DIR_LIGHT),
    m_dir(dir),
    m_spaceT(),
    m_sprite(nullptr),
    m_cBuffer()
{
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R16_TYPELESS;
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
        logger.err("Failed to create shadow map texture!");
        return;
    }

    //depth view of depthMap texture2d
    D3D11_DEPTH_STENCIL_VIEW_DESC dsv;
    dsv.Flags = 0;
    dsv.Format = DXGI_FORMAT_D16_UNORM;
    dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsv.Texture2D.MipSlice = 0;
    Graphics.getDevice()->CreateDepthStencilView(depthMap, &dsv, m_depthMapDsv.GetAddressOf());

    //shader view of depthMap texture2D for reading later
    D3D11_SHADER_RESOURCE_VIEW_DESC srv;
    srv.Format = DXGI_FORMAT_R16_UNORM;
    srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv.Texture2D.MipLevels = desc.MipLevels;
    srv.Texture2D.MostDetailedMip = 0;

    auto res = Graphics.getDevice()->CreateShaderResourceView(depthMap, &srv, m_depthMapSrv.GetAddressOf());

    XMFLOAT4 rightFloat4 = { 1.0f, 0, 0, 1.0f };
    XMVECTOR right = XMLoadFloat4(&rightFloat4);

    XMVECTOR _pos = XMLoadFloat4(&pos);
    XMVECTOR _dir = XMLoadFloat4(&dir);
    _dir = XMVector3Normalize(_dir);

    XMStoreFloat4(&dir, _dir); // store for up check

    XMFLOAT4 upFloat4 = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
    if (dir.y == -1.0f || dir.y == 1.0f)
    {
        upFloat4 = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    XMVECTOR up = XMLoadFloat4(&upFloat4);

    m_projection = XMMatrixOrthographicLH(2048, 2048, 0.1f, 1000.0f);

    XMVECTOR eyePos = XMLoadFloat4(&pos);
    XMVECTOR dirVec = XMLoadFloat4(&dir);
    XMVECTOR lookPos = XMVectorAdd(eyePos, dirVec);

    m_view = XMMatrixLookAtLH(eyePos, lookPos, up);
    m_space = XMMatrixMultiply(m_view, m_projection);
    m_spaceT = XMMatrixTranspose(m_space);

    m_shader = Graphics.getShader(L"SpotLight");

    m_cBuffer.dirLightSpace = m_spaceT;
    m_cBuffer.color = color;
    m_cBuffer.pos = pos;
    m_cBuffer.direction = dir;

    m_sprite = new Sprite("textures\\sprites\\003-sun.png", { m_pos.x, m_pos.y, m_pos.z }, m_color);
}

DirectionalLight::~DirectionalLight()
{

}

void DirectionalLight::bindShadowMap()
{
    auto context = Graphics.getContext();
    context->RSSetViewports(1, &m_viewport);
    ID3D11RenderTargetView* renderTargets[1] = { 0 };
    context->OMSetRenderTargets(1, renderTargets, m_depthMapDsv.Get());
    context->ClearDepthStencilView(m_depthMapDsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DirectionalLight::use(u32 slot)
{
    Graphics.m_gBuffer.dirLight.sun = getCBuffer();
    Graphics.getContext()->PSSetShaderResources(33, 1, m_depthMapSrv.GetAddressOf());
}

void DirectionalLight::renderShadowMap(Scene* scene)
{
    Graphics.getContext()->ClearDepthStencilView(m_depthMapDsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
    Graphics.setShadowRasterizer(true);
    bindShadowMap();
    m_shader->use();
    m_shader->bindModelMatrix(XMMatrixIdentity());

    m_spaceT = XMMatrixTranspose(m_space);

    m_cBuffer.dirLightSpace = m_spaceT;

    LightSpaceBuffer buffer = {
        m_spaceT,
        m_color,
        m_pos
    };

    m_shader->bindModelMatrix(XMMatrixIdentity());
    m_shader->bindCBuffer(&buffer);

    scene->renderMeshes();
    scene->renderEntities(m_shader);
    Graphics.resetRenderTarget();
    Graphics.resetViewport();
    Graphics.setShadowRasterizer(false);
}

AABB DirectionalLight::getBounds()
{
    return AABB(
        XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX),
        XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX)
    );
}

void DirectionalLight::draw()
{
    if (m_sprite)
    {
        m_sprite->draw();
    }
}

const SunBuffer& DirectionalLight::getCBuffer()
{
    return m_cBuffer;
}