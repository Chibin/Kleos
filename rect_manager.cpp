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

RectDynamicArray *CreateRectDynamicArray(uint32 size = 15000)
{
    auto *rda =
        static_cast<RectDynamicArray *>(malloc(sizeof(RectDynamicArray)));
    memset(rda, 0, sizeof(RectDynamicArray));
    rda->allocatedSize = size;
    rda->rects =
        static_cast<Rect **>(malloc(sizeof(Rect) * rda->allocatedSize));

    return rda;
}

RectManager *CreateRectManager()
{
    RectManager *rm = nullptr;
    rm = static_cast<RectManager *>(malloc(sizeof(RectManager)));
    rm->rda[0] = *CreateRectDynamicArray();
    rm->rda[1] = *CreateRectDynamicArray();
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
        fprintf(stderr, "I should not get here\n");
        printf("I should not get here\n");
        printf("Pausing all activity. Press enter in console to resume.\n");
        getchar();
        // getchar();
    }

    rda->rects[rda->size] = rect;
    rda->size++;
}
