#pragma once

#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
public:
    Camera();
    ~Camera();

    void setPosition(float x, float y, float z);
    void setRotation(float x, float y, float z);

    XMFLOAT3 getPosition();
    void move(XMFLOAT3 diff);

    const XMMATRIX& getView();

    void update();
private:

    XMFLOAT3 m_pos;
    XMFLOAT3 m_rotation;
    XMVECTOR m_look;

    XMMATRIX m_view;
};

