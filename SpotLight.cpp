#include "SpotLight.h"

#include "GraphicsManager.h"
#include "SystemManager.h"
#include "Shader.h"
#include "Scene.h"
#include "Sprite.h"

#include "MapFile_Parse.h"

SpotLight* SpotLight::Create(MF_Entity* entity)
{
    MF_Vector3 pos;
    MF_Vector4 color;
    MF_Vector3 dir;
    f32 height;
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

    if (!MF_GetAttributeVec3(entity, "dir", &dir))
    {
        return nullptr;
    }

    if (!MF_GetAttributeFloat(entity, "height", &height))
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

    return new SpotLight(
        MF3_TO_XM4(pos),
        MFCOL_TO_XM4(color),
        SHADOW_RES, SHADOW_RES,
        MF3_TO_XM4(dir),
        height,
        radius,
        cutoff
    );
}

SpotLight::SpotLight(
    XMFLOAT4 pos,
    XMFLOAT4 color,
    u32 width,
    u32 height,
    XMFLOAT4 dir,
    f32 length,
    f32 radius,
    f32 cutoff
) :
    ILight(pos, color, width, height, SPOT_LIGHT),
    m_dir(dir),
    m_length(length),
    m_radius(radius),
    m_cutoff(cutoff)
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

    //depth view of depthMap texture2d
    D3D11_DEPTH_STENCIL_VIEW_DESC dsv;
    dsv.Flags = 0;
    dsv.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsv.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsv.Texture2D.MipSlice = 0;
    Graphics.getDevice()->CreateDepthStencilView(depthMap, &dsv, m_depthMapDsv.GetAddressOf());

    //shader view of depthMap texture2D for reading later
    D3D11_SHADER_RESOURCE_VIEW_DESC srv;
    srv.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srv.Texture2D.MipLevels = desc.MipLevels;
    srv.Texture2D.MostDetailedMip = 0;
    
    Graphics.getDevice()->CreateShaderResourceView(depthMap, &srv, m_depthMapSrv.GetAddressOf());

    XMFLOAT4 rightFloat4 = { 1.0f, 0, 0, 1.0f };
    XMVECTOR right = XMLoadFloat4(&rightFloat4);

    XMVECTOR _pos = XMLoadFloat4(&pos);
    XMVECTOR _dir = XMLoadFloat4(&dir);
    _dir = XMVector3Normalize(_dir);
    XMVECTOR left;
    
    XMFLOAT4 upFloat4 = { 0, 1.0f, 0, 1.0f };
    XMVECTOR up = XMLoadFloat4(&upFloat4);

    XMFLOAT3 rightDP;
    XMFLOAT3 upDP;
    XMVECTOR rightDPV = XMVector3Dot(_dir, right);
    XMVECTOR upDPV = XMVector3Dot(_dir, up);

    XMStoreFloat3(&rightDP, rightDPV);
    XMStoreFloat3(&upDP, upDPV);

    if (std::fabs(rightDP.x) > std::fabs(upDP.x))
    {
        left = XMVector3Cross(_dir, up);
    }
    else
    {
        left = XMVector3Cross(_dir, right);
    }

   // if (XMVector4Equal(_dir, right))
   // {
   //     left = XMVector3Cross(_dir, up);
    //}
    //else
    //{        
    //    left = XMVector3Cross(_dir, right);
   // }
    float maxCone = getMaxCone();

    XMVECTOR end = XMVectorScale(_dir, length);
    XMVECTOR edge = XMVectorAdd(end, XMVectorScale(left, maxCone));

    auto _angle = XMVector3Dot(_dir, edge);
    auto _length = XMVector3Length(edge);

    XMFLOAT3 angle;
    XMFLOAT3 edgeLength;

    XMStoreFloat3(&angle, _angle);
    XMStoreFloat3(&edgeLength, _length);

    m_cBuffer.cosFullAngle = angle.x / edgeLength.x;

    float radFullAngle = std::acosf(m_cBuffer.cosFullAngle);
    ///// Matrices /////
    m_projection = XMMatrixPerspectiveFovLH(radFullAngle * 2.0f, (f32)width/(f32)height, 0.1f, 1000.0f);

    XMVECTOR eyePos = XMLoadFloat4(&pos);
    XMVECTOR dirVec = XMLoadFloat4(&dir);
    XMVECTOR lookPos = XMVectorAdd(eyePos, dirVec);

    m_view = XMMatrixLookAtLH(eyePos, lookPos, up);
    m_space = XMMatrixMultiply(m_view, m_projection);
    m_spaceT = XMMatrixTranspose(m_space);

    m_shader = Graphics.getShader(L"SpotLight");

    m_cBuffer.lightSpace = m_spaceT;
    m_cBuffer.color = color;
    m_cBuffer.pos = pos;
    m_cBuffer.direction = dir;
    m_cBuffer.length = length;
    m_cBuffer.radius = radius;
    m_cBuffer.cutoff = cutoff;

    m_sprite = new Sprite("textures\\sprites\\002-spotlight.png", { m_pos.x, m_pos.y, m_pos.z }, m_color);
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

    m_spaceT = XMMatrixTranspose(m_space);

    m_cBuffer.lightSpace = m_spaceT;

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

AABB SpotLight::getBounds()
{
    // https://iquilezles.org/articles/diskbbox/

    /*
    // bounding box for a cone defined by points pa and pb, and radii ra and rb
    bound3 ConeAABB( in vec3 pa, in vec3 pb, in float ra, in float rb )
    {
      vec3 a = pb - pa;
      vec3 e = sqrt( 1.0 - a*a/dot(a,a) );
      return bound3( min( pa - e*ra, pb - e*rb ),
	             max( pa + e*ra, pb + e*rb ) );
    }

    //////////

    I'm using 0 for ra - a cone with a point for a tip and not a disk
    */

    f32 radius = getMaxCone();
    XMVECTOR pa = XMLoadFloat4(&m_pos);
    XMVECTOR ray = XMLoadFloat4(&m_dir);
    ray = XMVectorScale(ray, m_length);
    XMVECTOR pb = XMVectorAdd(pa, ray);

    XMVECTOR _a = XMVectorSubtract(pb, pa);
    XMFLOAT3 a;
    XMStoreFloat3(&a, _a);

    XMVECTOR _dotA = XMVector3Dot(_a, _a);
    XMVECTOR _aSqr = XMVectorMultiply(_a, _a);

    XMFLOAT3 dotAV;
    XMStoreFloat3(&dotAV, _dotA);
    f32 dotA = dotAV.x;

    /*XMFLOAT3 aSqr;
    /*XMStoreFloat3(&aSqr, _aSqr);*/

    XMVECTOR e = XMVectorSqrt(XMVectorSubtract(g_XMOne, XMVectorScale(_aSqr, 1.0f / dotA)));

    XMVECTOR _min = XMVectorMin(
        pa,
        XMVectorSubtract(pb, XMVectorScale(e, radius))
    );

    XMVECTOR _max = XMVectorMax(
        pa,
        XMVectorAdd(pb, XMVectorScale(e, radius))
    );

    XMFLOAT3 min, max;

    XMStoreFloat3(&min, _min);
    XMStoreFloat3(&max, _max);

    return AABB(min, max);
}

void SpotLight::draw()
{
    if (m_sprite)
    {
        m_sprite->draw();
    }
}

f32 SpotLight::getMaxCone()
{
    if (m_cutoff == 0.0f)
    {
        return 0.0f;
    }
    return m_radius / sqrtf(m_cutoff);
}


SpotLightBuffer SpotLight::getCBuffer()
{
    return m_cBuffer;
}