#ifndef __SCENE_NODE__
#define __SCENE_NODE__

struct SceneNode
{
    Rect *rect;

    SceneNode *northWest;
    SceneNode *northEast;
    SceneNode *southWest;
    SceneNode *southEast;
};

#endif
