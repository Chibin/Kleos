#ifndef __COLLISION__
#define __COLLISION__

#include <cstdio>

#include "bitmap.h"
#include "rectangle.h"
#include "scene_node.h"

struct AABB
{
    v2 center;
    f32 radius;
};

struct MinMax
{
    v2 min;
    v2 max;
};

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
    GetMinMax(aabb->center, aabb->radius, o_minMax);
}

inline void GetMinMax(Rect *rect, MinMax *o_minMax)
{
    o_minMax->min = rect->min;
    o_minMax->max = rect->max;
}

void GetMinMax(SceneNode *sn, MinMax *o_minMax)
{
    GetMinMax(sn->rect, o_minMax);
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
    /* This may just be a test to see if a raycast will hit an AABB */
    double tx1 = (rect->min.x - initialPos.x) * rayDInv.x;
    double tx2 = (rect->max.x - initialPos.x) * rayDInv.x;

    double tmin = MIN(tx1, tx2);
    double tmax = MAX(tx1, tx2);

    double ty1 = (rect->min.y - initialPos.y) * rayDInv.y;
    double ty2 = (rect->max.y - initialPos.y) * rayDInv.y;

    tmin = MAX(tmin, MIN(MIN(ty1, ty2), tmax));
    tmax = MIN(tmax, MAX(MAX(ty1, ty2), tmin));

    printf("RayD.x: %f RayD.y: %f \n", rayDInv.x, rayDInv.y);
    printf("tmin:%f tmax:%f\n", tmin, tmax);

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

b32 TestAABBAABB(SceneNode *sn, AABB *range)
{
    MinMax rangeMinMax = {};
    MinMax snMinMax = {};
    GetMinMax(range, &rangeMinMax);
    GetMinMax(sn, &snMinMax);

    return TestAABBAABB(&snMinMax, &rangeMinMax);
}

#endif
