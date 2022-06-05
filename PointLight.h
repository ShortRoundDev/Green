#pragma once
#include "ILight.h"

#include "GTypes.h"
#include "AABB.h"

constexpr u32 POINT_LIGHT = 0;

class Sprite;

struct PointLightBuffer
{
    XMMATRIX lightSpace[6];
    XMFLOAT4 color;
    XMFLOAT4 lightPos;
    float radius;
    float cutoff;
};

class PointLight : public ILight
{
public:
    PointLight(XMFLOAT4 pos, XMFLOAT4 color, u32 width, u32 height, f32 radius, f32 cutoff);
    ~PointLight();

    virtual void use(u32 slot);
    virtual void renderShadowMap(Scene* scene);

    PointLightBuffer getCbuffer();
    AABB getBounds();

    PointLightBuffer getCBuffer();

    void draw();

private:
    void bindShadowMap(u32 face);

    f32 m_radius;
    f32 m_cutoff;

    f32 maxDistance();

    PointLightBuffer m_cBuffer;

    Sprite* m_sprite;

    ComPtr<ID3D11DepthStencilView> m_shadowMapFacesDsv[6];

};

