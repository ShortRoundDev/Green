#pragma once

#include "ILight.h"

#include "AABB.h"

constexpr u32 SPOT_LIGHT = 1;

struct SpotLightBuffer
{
    XMMATRIX lightSpace;
    XMFLOAT4 color;
    XMFLOAT4 pos;
    XMFLOAT4 direction;
    float length;
    float radius;
    float cutoff;
    float cosFullAngle;
};

struct MF_Entity;

class Sprite;

class SpotLight : public ILight
{
public:

    static SpotLight* Create(MF_Entity* entity);

    SpotLight(
        XMFLOAT4 pos,
        XMFLOAT4 color,
        u32 width,
        u32 height,
        XMFLOAT4 dir,
        f32 length,
        f32 radius,
        f32 cutoff
    );
    ~SpotLight();

    virtual void use(u32 slot);
    virtual void renderShadowMap(Scene* scene);

    AABB getBounds();

    void draw();

    SpotLightBuffer getCBuffer();

private:
    void bindShadowMap();
    XMFLOAT4 m_dir;
    XMMATRIX m_spaceT;

    f32 m_length;
    f32 m_radius;
    f32 m_cutoff;
    
    SpotLightBuffer m_cBuffer;

    f32 getMaxCone();

    Sprite* m_sprite;
};

