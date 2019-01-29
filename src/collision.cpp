#ifndef __COLLISION__
#define __COLLISION__

#include <cstdio>

#include "bitmap.h"
#include "rectangle.h"

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

b32 TestAABBAABB(Rect *a, Rect *b)
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
#endif
