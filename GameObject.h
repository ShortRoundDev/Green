#pragma once

#include "GTypes.h"

#include <DirectXMath.h>

class GameManager;

using namespace DirectX;

class GameObject
{
public:
    GameObject(XMFLOAT3 pos, u64 typeId);
    GameObject(XMFLOAT3 pos, u64 typeId, GameManager* gameManager);
    ~GameObject();

    virtual void update();
    virtual void draw();

    u32 getId();
    u64 getType();

protected:
    XMFLOAT3 m_pos;
    u32 m_id;
    u64 m_typeId;
};
