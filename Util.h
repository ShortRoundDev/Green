#pragma once

#include "GTypes.h"

#include "DirectXMath.h"

using namespace DirectX;

bool RayPlane(
    const XMFLOAT3& p, const XMFLOAT3& q,
    const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& c,
    const XMFLOAT3& n,
    XMFLOAT3& v, f32& t
);