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

void SetAABB(RectStorage *rs)
{
    MinMax aabbMinMax = {};
    for(memory_index i = 0; i < rs->rda.size; i++)
    {
        Rect *rect = rs->rda.rects[i];
            v2 center = V2(rect->basePosition);
            v2 dim = v2{rect->width, rect->height};
            MinMax result = GetMinMax(center, dim);

            aabbMinMax.min.x = aabbMinMax.min.x > result.min.x ? result.min.x : aabbMinMax.min.x;
            aabbMinMax.min.y = aabbMinMax.min.y > result.min.y ? result.min.y : aabbMinMax.min.y;
            aabbMinMax.max.x = aabbMinMax.max.x < result.max.x ? result.max.x : aabbMinMax.max.x;
            aabbMinMax.max.y = aabbMinMax.max.y < result.max.y ? result.max.y : aabbMinMax.max.y;
    }
    rs->aabbMinMax = aabbMinMax;
}

RectDynamicArray *CreateRectDynamicArray(GameMemory *gm, uint32 size = 15000)
{
    auto *rda =
        static_cast<RectDynamicArray *>(AllocateMemory(gm, (sizeof(RectDynamicArray))));
    memset(rda, 0, sizeof(RectDynamicArray));
    rda->allocatedSize = size;
    rda->rects =
        static_cast<Rect **>(AllocateMemory(gm, (sizeof(Rect) * rda->allocatedSize)));

    return rda;
}

RectManager *CreateRectManager(GameMemory *gm)
{
    RectManager *rm = nullptr;
    rm = static_cast<RectManager *>(AllocateMemory(gm, (sizeof(RectManager))));
    for (memory_index i = 0; i < ARRAY_SIZE(rm->rs); i++)
    {
        rm->rs[i].rda = *CreateRectDynamicArray(gm);
    }
    return rm;
}

void DeleteRectDynamicArray(RectDynamicArray *rda)
{
    if (rda == nullptr)
    {
        return;
    }

    free(rda->rects);

    printf("freeing size: %d\n", rda->size);
    free(rda);
}

void PushBack(RectDynamicArray *rda, Rect *rect)
{
    if (rda->allocatedSize <= rda->size)
    {
        ASSERT(!"I should not get here\n");
        ASSERT(!"Pausing all activity. Press enter in console to resume.\n");
    }

    rda->rects[rda->size] = rect;
    rda->size++;
}
