#ifndef __COLLISION_H_
#define __COLLISION_H_

#include <cstdio>

#include "bitmap.h"
#include "rectangle.h"
#include "scene_node.h"

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

struct SceneNode
{
    AABB aabb;
    Rect *rect;

    SceneNode *northWest;
    SceneNode *northEast;
    SceneNode *southWest;
    SceneNode *southEast;
};

#endif
