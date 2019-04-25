#ifndef __SCENE_NODE_H_
#define __SCENE_NODE_H_

#include "collision.h"

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
