#ifndef __COLLISION__
#define __COLLISION__

#include "collision.h"

MinMax GetMinMax(v2 center, v2 dim)
{
    MinMax result = {};
    result.max = center + dim * 0.5f;
    result.min = center - dim * 0.5f;
    return result;
}

inline void GetMinMax(v2 center, f32 radius, v2 *o_min, v2 *o_max)
{
    *o_min = center - radius;
    *o_max = center + radius;
}

void GetMinMax(v2 center, f32 radius, MinMax *o_minMax)
{
    GetMinMax(center, radius, &o_minMax->min, &o_minMax->max);
}

inline void GetMinMax(AABB *aabb, MinMax *o_minMax)
{
    *o_minMax = GetMinMax(aabb->center, 2.0f * aabb->halfDim);
}

inline void GetMinMax(Rect *rect, MinMax *o_minMax)
{
    o_minMax->min = rect->min;
    o_minMax->max = rect->max;
}

inline MinMax GetMinMax(Rect *rect)
{
    MinMax result = {};
    result.min = rect->min;
    result.max = rect->max;
    return result;
}

b32 ContainsPoint(Rect *rect, v2 p)
{
    MinMax minMax = GetMinMax(rect);
    return minMax.min.x <= p.x && p.x <= minMax.max.x && minMax.min.y <= p.y && p.y <= minMax.max.y;
}

b32 ContainsPoint(AABB *aabb, v2 p)
{
    MinMax minMax = {};
    GetMinMax(aabb, &minMax);

    return minMax.min.x <= p.x && p.x <= minMax.max.x && minMax.min.y <= p.y && p.y <= minMax.max.y;
}

/* RayCast Intersection */
bool IntersectionAABB(Rect *rect, v2 initialPos, glm::vec3 rayDInv)
{
    /* Test to see if a raycast will hit an AABB */
    double tx1 = (rect->min.x - initialPos.x) * rayDInv.x;
    double tx2 = (rect->max.x - initialPos.x) * rayDInv.x;

    double tmin = MIN(tx1, tx2);
    double tmax = MAX(tx1, tx2);

    double ty1 = (rect->min.y - initialPos.y) * rayDInv.y;
    double ty2 = (rect->max.y - initialPos.y) * rayDInv.y;

    tmin = MAX(tmin, MIN(MIN(ty1, ty2), tmax));
    tmax = MIN(tmax, MAX(MAX(ty1, ty2), tmin));

#if 0
    printf("RayD.x: %f RayD.y: %f \n", rayDInv.x, rayDInv.y);
    printf("tmin:%f tmax:%f\n", tmin, tmax);
#endif

    return tmax > MAX(tmin, 0.0);
}

b32 TestAABBAABB(MinMax *a, MinMax *b)
{
    if (a->max.v[0] < b->min.v[0] || a->min.v[0] > b->max.v[0])
    {
        return false;
    }
    if (a->max.v[1] < b->min.v[1] || a->min.v[1] > b->max.v[1])
    {
        return false;
    }

    return true;
}

b32 TestAABBAABB(Rect *a, Rect *b)
{
    MinMax aMinMax = {};
    MinMax bMinMax = {};
    GetMinMax(a, &aMinMax);
    GetMinMax(b, &bMinMax);

    return TestAABBAABB(&aMinMax, &bMinMax);
}

b32 TestAABBAABB(AABB *a, AABB *b)
{
    MinMax aMinMax = {};
    MinMax bMinMax = {};
    GetMinMax(a, &aMinMax);
    GetMinMax(b, &bMinMax);

    return TestAABBAABB(&aMinMax, &bMinMax);
}
#endif
