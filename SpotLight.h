#pragma once

#include "ILight.h"

class SpotLight : public ILight
{
public:
    SpotLight(XMFLOAT4 pos, XMFLOAT4 color, u32 width, u32 height, XMFLOAT4 dir);
    ~SpotLight();

    virtual void use(u32 slot);
    virtual void renderShadowMap(Scene* scene);

private:
    void bindShadowMap();
    XMFLOAT4 m_dir;
};

