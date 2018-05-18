#ifndef __MATH__
#define __MATH__

#include "sdl_common.h"
#include <cinttypes>
#include <cstring>

#ifndef UINT32_MAX
#define UINT32_MAX 0xFFFFFFFF
#endif

#ifndef INT32_MAX
#define INT32_MAX 0x7FFFFFFF
#endif

#define FLOOR(value) int(value)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define KILOBYTE(x) 1024 * x
#define MEGABYTE(x) KILOBYTE(1024) * x
#define GIGABYTE(x) MEGABYTE(1024) * x

#define ZeroStruct(x) ZeroSize(&x, sizeof(x))

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int8 s8;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;

typedef float real32;
typedef double real64;

typedef float f32;
typedef double f64;
typedef double d64;

typedef bool b32;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef size_t memory_index;

typedef uintptr_t umm;
typedef intptr_t smm;

union v2 {
    struct
    {
        real32 x;
        real32 y;
    };
    real32 v[2];
};

union v3 {
    struct
    {
        real32 x;
        real32 y;
        real32 z;
    };

    struct
    {
        real32 r;
        real32 g;
        real32 b;
    };
    real32 v[3];
};

union v4 {
    struct
    {
        real32 x;
        real32 y;
        real32 z;
        real32 w;
    };

    struct
    {
        real32 r;
        real32 g;
        real32 b;
        real32 a;
    };
    real32 v[4];
};

inline v3 operator+(v3 A, v3 B)
{

    A.x += B.x;
    A.y += B.y;
    A.z += B.z;

    return A;
}

inline v3 &operator+=(v3 &A, v3 B)
{

    A.x += B.x;
    A.y += B.y;
    A.z += B.z;

    return A;
}

inline v4 operator+(v4 A, v4 B)
{

    A.x += B.x;
    A.y += B.y;
    A.z += B.z;
    A.w += B.w;

    return A;
}

inline v4 operator*(v4 A, v4 B)
{

    A.x *= B.x;
    A.y *= B.y;
    A.z *= B.z;
    A.w *= B.w;

    return A;
}

inline v4 &operator/(v4 &A, v4 B)
{

    A.x /= B.x;
    A.y /= B.y;
    A.z /= B.z;
    A.w /= B.w;

    return A;
}

inline v4 operator-(v4 a, v4 b)
{

    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;

    return a;
}

inline v4 &operator/=(v4 &A, v4 B)
{

    A.x *= B.x;
    A.y *= B.y;
    A.z *= B.z;
    A.w *= B.w;

    return A;
}

inline v4 &operator*=(v4 &A, v4 B)
{

    A.x *= B.x;
    A.y *= B.y;
    A.z *= B.z;
    A.w *= B.w;

    return A;
}

inline v4 &operator-=(v4 &A, v4 B)
{

    A.x -= B.x;
    A.y -= B.y;
    A.z -= B.z;
    A.w -= B.w;

    return A;
}

inline v4 &operator+=(v4 &A, v4 B)
{

    A.x += B.x;
    A.y += B.y;
    A.z += B.z;
    A.w += B.w;

    return A;
}

union m4 {
    struct rv
    {
        v4 a1;
        v4 a2;
        v4 a3;
        v4 a4;
    };
    real32 v[16];
};

inline void ZeroSize(void *_data, memory_index size)
{
    u8 *base = (u8 *)_data;
    while (size--)
    {
        *base++ = 0;
    }
}

inline u32 SafeCastToU32(memory_index x)
{
    ASSERT(x <= UINT32_MAX);
    return (u32)x;
}

inline u8 SafeCastToU8(f32 num)
{
    ASSERT(num <= 256 && num >= 0);
    return (u8)num;
}

inline s32 *SafeCastU32ToS32(u32 *u32Pointer)
{

    ASSERT(*u32Pointer <= INT32_MAX);
    return (s32 *)u32Pointer;
}
#endif
