#ifndef __MATH__
#define __MATH__

#include "sdl_common.h"

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#ifndef UINT32_MAX
#define UINT32_MAX 0xFFFFFFFF
#endif

#ifndef INT32_MAX
#define INT32_MAX 0x7FFFFFFF
#endif

#define FLOOR(value) int(value)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define KILOBYTE(x) (1024 * (x))
#define MEGABYTE(x) KILOBYTE(1024) * (x)
#define GIGABYTE(x) MEGABYTE(1024) * (x)

#define ZeroStruct(x) ZeroSize(&(x), sizeof(x))

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
        f32 x;
        f32 y;
    };
    struct
    {
        f32 width;
        f32 height;
    };
    f32 v[2];
};

struct v2i
{
    union
    {
        struct
        {
            s32 x;
            s32 y;
        };
        s32 v[2];
    };
};

union v3 {
    struct
    {
        f32 x;
        f32 y;
        f32 z;
    };

    struct
    {
        f32 r;
        f32 g;
        f32 b;
    };
    f32 v[3];
};

union v4 {
    struct
    {
        f32 x;
        f32 y;
        f32 z;
        f32 w;
    };

    struct
    {
        f32 r;
        f32 g;
        f32 b;
        f32 a;
    };
    f32 v[4];
};

union m4 {
    struct
    {
        union v4 a1;
        union v4 a2;
        union v4 a3;
        union v4 a4;
    };
    f32 v[16];
};

void ZeroSize(void *_data, memory_index size);
u32 SafeCastToU32(memory_index x);
u8 SafeCastToU8(f32 num);
s32 *SafeCastU32ToS32(u32 *u32Pointer);
v2 operator-(v2 A, v2 B);
v2 operator-(v2 A, f32 B);
v2 operator+(v2 A, v2 B);
v2 operator+(f32 A, v2 B);
v2 operator+(v2 A, f32 B);
v2 operator*(v2 A, f32 B);
v2 operator*(f32 A, v2 B);
v2 operator/(f32 A, v2 B);
v2 operator/(v2 A, f32 B);
v2 &operator-=(v2 &A, v2 B);
v2 &operator+=(v2 &A, v2 B);
v3 operator+(v3 A, v3 B); //NOLINT
v3 operator+(f32 A, v3 B);
v3 operator+(v3 A, f32 B);
v3 &operator-=(v3 &A, v3 B);
v3 &operator+=(v3 &A, v3 B);
b32 operator<(v2 A, v2 B); //NOLINT
b32 operator>(v2 A, v2 B); //NOLINT
v2 V2(v3 A);
v2 abs(v2);
v3 V3(v2 A, f32 B);

#endif
