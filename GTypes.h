#pragma once

#include <stdint.h>
#include <typeinfo>

typedef int8_t i8;
typedef uint8_t u8;

typedef int16_t i16;
typedef uint16_t u16;

typedef int32_t i32;
typedef uint32_t u32;

typedef int64_t i64;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

constexpr f32 M_PI_F = 3.14159265358979323846f;
constexpr f64 M_PI_D = 3.14159265358979323846;

constexpr f32 M_PI_2_F = M_PI_F / 2.0f;
constexpr f64 M_PI_2_D = M_PI_D / 2.0;

constexpr f32 M_PI_4_F = M_PI_F / 4.0f;
constexpr f64 M_PI_4_D = M_PI_D / 4.0;

constexpr float TO_RAD = 0.0174532925f;

#define TYPE_ID(x) (u64)((typeid(x).hash_code()))

#define MF3_TO_XM3(a) (XMFLOAT3(-(a).x, (a).z, -(a).y))
#define MF3_TO_XM4(a) (XMFLOAT4(-(a).x, (a).z, -(a).y, 1.0f))
#define MF4_TO_XM4(a) (XMFLOAT4(-(a).x, (a).z, -(a).y, 1.0f))
#define MFCOL_TO_XM4(a) (XMFLOAT4((a).x / 255.0f, (a).y / 255.0f, (a).z / 255.0f, (a).w / 255.0f))