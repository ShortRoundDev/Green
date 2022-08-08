#include "Util.h"


bool RayPlane(
    const XMFLOAT3& p, const XMFLOAT3& q,
    const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& c,
    const XMFLOAT3& n,
    XMFLOAT3& v, f32& t
)
{
    XMVECTOR _p = XMLoadFloat3(&p);
    XMVECTOR _q = XMLoadFloat3(&q);

    XMVECTOR _a = XMLoadFloat3(&a);
    XMVECTOR _b = XMLoadFloat3(&b);
    XMVECTOR _c = XMLoadFloat3(&c);

    XMVECTOR _n = XMLoadFloat3(&n);

    XMVECTOR ab = XMVectorSubtract(_b, _a);
    XMVECTOR ac = XMVectorSubtract(_c, _a);
    XMVECTOR qp = XMVectorSubtract(_p, _q);

    f32 d = XMVectorGetX(XMVector3Dot(qp, _n));
    if (d <= 0.0f)
    {
        return false;
    }

    XMVECTOR ap = XMVectorSubtract(_p, _a);
    t = XMVectorGetX(XMVector3Dot(ap, _n));
    if (t <= 0.0f)
    {
        return false;
    }

    XMVECTOR e = XMVector3Cross(qp, ap);
    f32 _v = XMVectorGetX(XMVector3Dot(ac, e));
    if (_v <= 0.0f || _v > d)
    {
        return false;
    }

    f32 _w = -XMVectorGetX(XMVector3Dot(ab, e));
    if (_w < 0.0f || _v + _w > d)
    {
        return false;
    }

    f32 ood = 1.0f / d;
    t *= ood;
    _v = _v * ood;
    _w = _w * ood;
    auto _u = 1.0f - _v - _w;

    XMVECTOR outV = XMVectorAdd(
        XMVectorAdd(
            XMVectorScale(_a, _u),
            XMVectorScale(_b, _v)
        ),
        XMVectorScale(_c, _w)
    );

    XMStoreFloat3(&v, outV);
    
    return true;
}

void convertAiMatrixToXMMatrix(const aiMatrix4x4& aiMat, XMMATRIX& xmMat)
{
    xmMat = XMMatrixSet(
        aiMat.a1, aiMat.a2, aiMat.a3, aiMat.a4,
        aiMat.b1, aiMat.b2, aiMat.b3, aiMat.b4,
        aiMat.c1, aiMat.c2, aiMat.c3, aiMat.c4,
        aiMat.d1, aiMat.d2, aiMat.d3, aiMat.d4
    );
}
