#pragma once
#include "ILight.h"

#include "GlobalBuffer.h"

constexpr u32 DIR_LIGHT = 3;

struct MF_Entity;
class Sprite;

class DirectionalLight : public ILight
{
public:
    static DirectionalLight* Create(MF_Entity* entity);

    DirectionalLight(XMFLOAT4 pos, XMFLOAT4 color, XMFLOAT4 dir, u32 width, u32 height);
    virtual ~DirectionalLight();

    virtual void use(u32 slot);
    virtual void renderShadowMap(Scene* scene);

    const SunBuffer& getCBuffer();
    AABB getBounds();

    void draw();

private:
    void bindShadowMap();
    
    XMFLOAT4 m_dir;
    XMMATRIX m_spaceT;

    SunBuffer m_cBuffer;
    Sprite* m_sprite;
};

