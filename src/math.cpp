#include "math.h"

/*
 * V2
 */

v2 operator-(v2 A) //NOLINT
{

    v2 result;
    result.x = -A.x;
    result.y = -A.y;

    return result;
}

v2 operator-(v2 A, v2 B) //NOLINT
{
    v2 result;
    result.x = A.x - B.x;
    result.y = A.y - B.y;

    return result;
}

v2 &operator-=(v2 &A, v2 B)
{

    A.x -= B.x;
    A.y -= B.y;

    return A;
}

v2 operator+(v2 A, v2 B) //NOLINT
{

    v2 result;
    result.x = A.x + B.x;
    result.y = A.y + B.y;

    return result;
}

v2 operator+(v2 A, f32 B) //NOLINT
{

    v2 result;
    result.x = A.x + B;
    result.y = A.y + B;

    return result;
}

v2 &operator+=(v2 &A, v2 B)
{

    A.x += B.x;
    A.y += B.y;

    return A;
}

v2 operator*(v2 A, f32 B) //NOLINT
{

    A.x *= B;
    A.y *= B;

    return A;
}

v2 operator*(f32 A, v2 B) //NOLINT
{

    B.x *= A;
    B.y *= A;

    return B;
}

v2 operator+(f32 A, v2 B) //NOLINT
{

    B.x += A;
    B.y += A;

    return B;
}

v2 operator-(v2 A, f32 B) //NOLINT
{

    A.x -= B;
    A.y -= B;

    return A;
}

v2 operator/(v2 A, f32 B) //NOLINT
{

    A.x /= B;
    A.y /= B;

    return A;
}

v2 operator/(f32 A, v2 B) //NOLINT
{

    B.x /= A;
    B.y /= A;

    return B;
}

b32 operator<(v2 A, v2 B) //NOLINT
{

    if ((A.x < B.x) && (A.y < B.y))
    {
        return true;
    }

    return false;
}

v2 V2(v3 A)
{
    return v2{A.x, A.y};
}

v2 abs(v2 a)
{
    return v2{fabsf(a.x), fabsf(a.y)};
}

/*
 * V3
 */

v3 operator+(v3 A, v3 B) //NOLINT
{

    A.x += B.x;
    A.y += B.y;
    A.z += B.z;

    return A;
}

v3 &operator+=(v3 &A, v3 B)
{

    A.x += B.x;
    A.y += B.y;
    A.z += B.z;

    return A;
}

v3 operator-(v3 A, v3 B) //NOLINT
{

    A.x -= B.x;
    A.y -= B.y;
    A.z -= B.z;

    return A;
}

v3 &operator-=(v3 &A, v3 B)
{

    A.x -= B.x;
    A.y -= B.y;
    A.z -= B.z;

    return A;
}

v3 V3(v2 A, f32 B)
{
    return v3{A.x, A.y, B};
}

/*
 * V4
 */

v4 operator+(v4 A, v4 B)
{

    A.x += B.x;
    A.y += B.y;
    A.z += B.z;
    A.w += B.w;

    return A;
}

v4 operator*(v4 A, v4 B)
{

    A.x *= B.x;
    A.y *= B.y;
    A.z *= B.z;
    A.w *= B.w;

    return A;
}

v4 &operator/(v4 &A, v4 B) // NOLINT
{

    A.x /= B.x;
    A.y /= B.y;
    A.z /= B.z;
    A.w /= B.w;

    return A;
}

v4 operator-(v4 a, v4 b)
{

    a.x -= b.x;
    a.y -= b.y;
    a.z -= b.z;
    a.w -= b.w;

    return a;
}

v4 &operator/=(v4 &A, v4 B)
{

    A.x *= B.x;
    A.y *= B.y;
    A.z *= B.z;
    A.w *= B.w;

    return A;
}

v4 &operator*=(v4 &A, v4 B)
{

    A.x *= B.x;
    A.y *= B.y;
    A.z *= B.z;
    A.w *= B.w;

    return A;
}

v4 &operator-=(v4 &A, v4 B)
{

    A.x -= B.x;
    A.y -= B.y;
    A.z -= B.z;
    A.w -= B.w;

    return A;
}

v4 &operator+=(v4 &A, v4 B)
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
u32 SafeCastToU32(f32 x)
{
    /* We know that data will be lost if we do this */
    ASSERT(x > -0);
    ASSERT(x <= UINT32_MAX); // NOLINT
    return (u32)x;
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

u8 SafeCastToU8(s32 num)
{
    ASSERT(num <= 256 && num >= 0); // NOLINT
    return (u8)num;
}

s32 *SafeCastU32ToS32(u32 *u32Pointer)
{
    ASSERT(*u32Pointer <= INT32_MAX); // NOLINT
    return (s32 *)u32Pointer;
}
