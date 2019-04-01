#include "define.h"
#include "game_memory.h"
#include "logger.h"
#include "rectangle.h"
#include <cstdio>

struct RectDynamicArray
{
    int size;
    int allocatedSize; /*not in bytes but amount of entities */

    Rect **rects;
};

#pragma warning(push)
#pragma warning(disable : 4201)
struct RectManager
{

    union {
        struct
        {
            RectDynamicArray Traversable;
            RectDynamicArray NonTraversable;
        };

        RectDynamicArray rda[2];
    };

    Rect *player;
};
#pragma warning(pop)

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
    for (memory_index i = 0; i < ARRAY_SIZE(rm->rda); i++)
    {
        rm->rda[i] = *CreateRectDynamicArray(gm);
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
