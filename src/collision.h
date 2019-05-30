#ifndef __COLLISION_H_
#define __COLLISION_H_

#include <cstdio>

#include "bitmap.h"
#include "rectangle.h"

struct AABB
{
    v2 center;
    v2 halfDim;
};

struct MinMax
{
    v2 min;
    v2 max;
};

MinMax GetMinMax(v2 center, v2 dim);
b32 TestAABBAABB(MinMax *a, MinMax *b);

#endif
