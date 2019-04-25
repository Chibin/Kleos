#ifndef __SCENE_NODE__
#define __SCENE_NODE__

#include "scene_node.h"

void GetMinMax(SceneNode *sn, MinMax *o_minMax)
{
    GetMinMax(sn->rect, o_minMax);
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
