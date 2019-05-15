#ifndef __RECT_MANAGER_H__
#define __RECT_MANAGER_H__

#include "define.h"
#include "game_memory.h"
#include "logger.h"
#include "rectangle.h"
#include <cstdio>

#include "collision.h"

struct RectDynamicArray
{
    int size;
    int allocatedSize; /*not in bytes but amount of entities */

    Rect **rects;
};

struct RectStorage
{
    /*The smallest and largest points of the all rects in the dyanmic array */
    MinMax aabbMinMax;
    RectDynamicArray rda;
};


#pragma warning(push)
#pragma warning(disable : 4201)
struct RectManager
{

    union {
        struct
        {
            RectStorage Traversable;
            RectStorage NonTraversable;
        };

        RectStorage rs[2];
    };

    Rect *player;
};
#pragma warning(pop)

#endif
