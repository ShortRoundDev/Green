#include "Frustum.h"

Frustum::Frustum() : m_planes()
{

}

void Frustum::load(f32 depth, XMMATRIX projection, XMMATRIX view)
{
    f32 zMin;
    f32 r;

    XMMATRIX mat;
    XMFLOAT4X4 _projection;
    XMFLOAT4X4 _view;

    XMStoreFloat4x4(&_projection, projection);
    XMStoreFloat4x4(&_view, view);

    zMin = -_projection._43 / _projection._33;
    r = depth / (depth - zMin);

    _projection._33 = r;
    _projection._43 = -r * zMin;

    projection = XMLoadFloat4x4(&_projection);
    mat = XMMatrixMultiply(view, projection);

    XMFLOAT4X4 _mat;
    XMStoreFloat4x4(&_mat, mat);

    XMFLOAT4 plane;
    plane.x = _mat._14 + _mat._13;
    plane.y = _mat._24 + _mat._23;
    plane.z = _mat._34 + _mat._33;
    plane.w = _mat._44 + _mat._43;
    m_planes[0] = XMLoadFloat4(&plane);

    plane.x = _mat._14 - _mat._13;
    plane.y = _mat._24 - _mat._23;
    plane.z = _mat._34 - _mat._33;
    plane.w = _mat._44 - _mat._43;
    m_planes[1] = XMLoadFloat4(&plane);

    plane.x = _mat._14 + _mat._11;
    plane.y = _mat._24 + _mat._21;
    plane.z = _mat._34 + _mat._31;
    plane.w = _mat._44 + _mat._41;
    m_planes[2] = XMLoadFloat4(&plane);

    plane.x = _mat._14 - _mat._11;
    plane.y = _mat._24 - _mat._21;
    plane.z = _mat._34 - _mat._31;
    plane.w = _mat._44 - _mat._41;
    m_planes[3] = XMLoadFloat4(&plane);

    plane.x = _mat._14 - _mat._12;
    plane.y = _mat._24 - _mat._22;
    plane.z = _mat._34 - _mat._32;
    plane.w = _mat._44 - _mat._42;
    m_planes[4] = XMLoadFloat4(&plane);

    plane.x = _mat._14 + _mat._12;
    plane.y = _mat._24 + _mat._22;
    plane.z = _mat._34 + _mat._32;
    plane.w = _mat._44 + _mat._42;
    m_planes[5] = XMLoadFloat4(&plane);
}

Frustum::~Frustum()
{

}

XMVECTOR* Frustum::getPlanes()
{
    return m_planes;
}

bool Frustum::checkPoint(const XMFLOAT3& point)
{

    XMVECTOR pointV = XMLoadFloat3(&point);

    for(int i = 0; i < 6; i++)
    {
        if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], pointV)) < 0.0f)
        {
            return false;
        }
    }
    return true;
}

bool Frustum::checkBox(AABB* box)
{
    XMFLOAT3 points[2] = { box->getMin(), box->getMax() };
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            auto x = points[j].x;
            for (int k = 0; k < 2; k++)
            {
                auto y = points[k].y;
                for (int l = 0; l < 2; l++)
                {
                    auto z = points[l].z;

                    auto point = XMFLOAT3(x, y, z);
                    XMVECTOR pointV = XMLoadFloat3(&point);
                    if (XMVectorGetX(XMPlaneDotCoord(m_planes[i], pointV)) >= 0.0f)
                    {
                        goto NextPlane;
                    }
                }
            }
        }
        return false;

    NextPlane:
        continue;
    }
    return true;

}