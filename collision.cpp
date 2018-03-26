#ifndef __COLLISION__
#define __COLLISION__

/* RayCast Intersection */
bool IntersectionAABB(Rect *rect, v2 initialPos, glm::vec3 rayDInv)
{ 
    /* This may just be a test to see if a raycast will hit an AABB */
    double tx1 = (rect->minX - initialPos.x)*rayDInv.x;
    double tx2 = (rect->maxX - initialPos.x)*rayDInv.x;

    double tmin = MIN(tx1, tx2);
    double tmax = MAX(tx1, tx2);

    double ty1 = (rect->minY - initialPos.y)*rayDInv.y;
    double ty2 = (rect->maxY - initialPos.y)*rayDInv.y;

    tmin = MAX(tmin, MIN(MIN(ty1, ty2), tmax));
    tmax = MIN(tmax, MAX(MAX(ty1, ty2), tmin));

    printf("RayD.x: %f RayD.y: %f \n", rayDInv.x, rayDInv.y);
    printf("tmin:%f tmax:%f\n", tmin, tmax);

    return tmax > MAX(tmin, 0.0);
}

int TestAABBAABB(Rect *a, Rect *b)
{
    if (a->max[0] < b->min[0] || a->min[0] > b->max[0]) return false;
    if (a->max[1] < b->min[1] || a->min[1] > b->max[1]) return false;

    return true;
}
#endif
