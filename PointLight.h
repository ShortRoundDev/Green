#pragma once
#include "ILight.h"

struct PointLightBuffer
{
    XMMATRIX lightSpace[6];
    XMFLOAT4 color;
    XMFLOAT4 lightPos;
    float constantDropoff;
    float linearDropoff;
    float quadraticDropoff;
};

class PointLight : public ILight
{
public:
    PointLight(XMFLOAT4 pos, XMFLOAT4 color, u32 width, u32 height);
    ~PointLight();

    virtual void use(u32 slot);
    virtual void renderShadowMap(Scene* scene);

    const PointLightBuffer& getCbuffer();

private:
    void bindShadowMap(u32 face);

    ComPtr<ID3D11DepthStencilView> m_shadowMapFacesDsv[6];

    PointLightBuffer m_cBuffer;
};

