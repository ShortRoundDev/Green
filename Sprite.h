#pragma once

#include "Sprite.h"

#include "DirectXMath.h"

#include <string>

using namespace DirectX;

class Texture;
class Mesh;
class Shader;

struct SpriteBuffer
{
    XMFLOAT4 pos;
    XMFLOAT4 color;
    XMFLOAT2 scale;
};

class Sprite
{
public:
    Sprite(std::string textureName, XMFLOAT3 position, XMFLOAT4 color);
    ~Sprite();

    void draw();

    SpriteBuffer getCbuffer();
private:

    Shader* m_shader;
    Texture* m_texture;

    Mesh* m_mesh;

    SpriteBuffer m_cBuffer;
};

