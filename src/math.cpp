#include "math.h"

inline v3 operator+(v3 A, v3 B) //NOLINT
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

inline v4 &operator/(v4 &A, v4 B) // NOLINT
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

void ZeroSize(void *_data, memory_index size)
{
    auto *base = (u8 *)_data;
    while (size--)
    {
        *base++ = 0;
    }
}

u32 SafeCastToU32(memory_index x)
{
    ASSERT(x <= UINT32_MAX); // NOLINT
    return (u32)x;
}

u8 SafeCastToU8(f32 num)
{
    ASSERT(num <= 256 && num >= 0); // NOLINT
    return (u8)num;
}

s32 *SafeCastU32ToS32(u32 *u32Pointer)
{

    ASSERT(*u32Pointer <= INT32_MAX); // NOLINT
    return (s32 *)u32Pointer;
}
