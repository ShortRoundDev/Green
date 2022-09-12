#pragma once

#include "GTypes.h"

#include <DirectXMath.h>

#include <bit>

using namespace DirectX;

struct GVertex
{
    XMFLOAT3 pos;    //  0 - 11
    XMFLOAT3 normal; // 12 - 23
    XMFLOAT2 tex;    // 24 - 31
    XMINT4 bones;
    XMFLOAT4 weights;

    GVertex() :
        pos(0, 0, 0),
        normal(0, 0, 0),
        tex(0, 0),
        bones(0, 0, 0, 0),
        weights(0, 0, 0, 0)
    {

    }
#pragma region Constructors
    GVertex(
        XMFLOAT3 pos,
        XMFLOAT3 normal,
        XMFLOAT2 tex,
        XMINT4 bones,
        XMFLOAT4 weights
    ) :
        pos(pos),
        normal(normal),
        tex(tex),
        bones(bones),
        weights(weights)
    {

    }

    GVertex(float x, float y, float z, float nX, float nY, float nZ, float u, float v, XMINT4 _bones = { -1, -1, -1, -1}, XMFLOAT4 _weights = { 0.0f, 0.0f, 0.0f, 0.0f }) :
        pos(x, y, z),
        normal(nX, nY, nZ),
        tex(u, v),
        bones(_bones),
        weights(_weights)
    {
    }
#pragma endregion
};

bool operator == (const GVertex& a, const GVertex& b);

struct HashGVertex
{
public:
    sz operator()(const GVertex& vertex) const
    {
        //randomly generated prime numbers of 8 digits https://bigprimes.org/
        return (std::bit_cast<u64>((f64)vertex.pos.x)       * 73856093ul) ^
               (std::bit_cast<u64>((f64)vertex.pos.y)       * 19349663ul) ^
               (std::bit_cast<u64>((f64)vertex.pos.z)       * 83492791ul) ^
               (std::bit_cast<u64>((f64)vertex.normal.x)    * 59295539ul) ^
               (std::bit_cast<u64>((f64)vertex.normal.y)    * 72444347ul) ^
               (std::bit_cast<u64>((f64)vertex.normal.z)    * 19759969ul) ^
               (std::bit_cast<u64>((f64)vertex.tex.x)       * 51405503ul) ^
               (std::bit_cast<u64>((f64)vertex.tex.y)       * 72923369ul) ^
               ((u64)vertex.bones.x                         * 20953679ul) ^
               ((u64)vertex.bones.y                         * 96761563ul) ^
               ((u64)vertex.bones.z                         * 11230361ul) ^
               ((u64)vertex.bones.w                         * 69894679ul) ^
               ((std::bit_cast<u64>((f64)vertex.weights.x)) * 36070649ul) ^
               ((std::bit_cast<u64>((f64)vertex.weights.y)) * 88282429ul) ^
               ((std::bit_cast<u64>((f64)vertex.weights.z)) * 21218507ul) ^
               ((std::bit_cast<u64>((f64)vertex.weights.w)) * 83929007ul);
    }
};

struct EqualsGVertex
{
public:
    bool operator()(const GVertex& a, const GVertex& b) const
    {
        return a == b;
    }
};