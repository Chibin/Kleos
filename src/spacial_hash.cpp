#ifndef __SPACIAL_HASH__
#define __SPACIAL_HASH__

#include "../src/game_memory.h"
#include "../src/array.h"
#include "../src/hash_set.cpp"

struct SpatialStore
{
    Rect **rects;
};

struct SpacialHash
{
    SpatialStore *buckets;
    memory_index bucketCount;
    memory_index cellGrid;
    memory_index bucketsPerColumn;
    struct GameMemory *gm;
};

SpacialHash *CreateSpacialHash(
        GameMemory *gm,
        memory_index bucketCount,
        memory_index bucketsPerColumn,
        memory_index cellGrid)
{
    SpacialHash *result = (SpacialHash *)AllocateMemory0(gm, sizeof(SpacialHash));
    result->buckets = (SpatialStore *)AllocateMemory0(gm, sizeof(SpatialStore) * bucketCount);
    result->cellGrid = cellGrid;
    result->bucketsPerColumn = bucketsPerColumn;
    result->gm = gm;
    return result;
}


/* convert a point to a possible bucket*/
memory_index SpacialHashPointToBucket(SpacialHash *sh, v2 point)
{
    /* turn buckets into a 2D grid and see if the point exist in that grid */
    /* Find an easy way to put a point into a grid */
    if (point.x < 0)
        printf("x value is negative using fabsf\n");
    if (point.y < 0)
        printf("y value is negative using fabsf\n");

    u8 xGrid = SafeCastToU8(fabsf(point.x) / sh->cellGrid);
    u8 yGrid = SafeCastToU8(fabsf(point.y) / sh->cellGrid);
    memory_index hashId = xGrid + yGrid * sh->bucketsPerColumn;

    return hashId;
}

memory_index *SpacialHashRectToKey(SpacialHash *sh, Rect *rect)
{

    ARRAY_CREATE(memory_index, sh->gm, arr);

#if 0
    v2 bottomLeft  = rect->min;
    v2 bottomRight  = v2{rect->max.x, rect->min.y};
    v2 topLeft = v2{rect->min.x, rect->max.y};
    v2 topRight = rect->max;

    ARRAY_PUSH(memory_index, sh->gm, arr, SpacialHashPointToBucket(sh, topRight));
    ARRAY_PUSH(memory_index, sh->gm, arr, SpacialHashPointToBucket(sh, bottomLeft));
    ARRAY_PUSH(memory_index, sh->gm, arr, SpacialHashPointToBucket(sh, topLeft));
    ARRAY_PUSH(memory_index, sh->gm, arr, SpacialHashPointToBucket(sh, bottomRight));
#endif

    for(f32 i = rect->min.x; i < rect->max.x + sh->cellGrid; i += sh->cellGrid)
    {
        for(f32 j = rect->min.y; j < rect->max.y + sh->cellGrid; j += sh->cellGrid)
        {
            if(i > rect->max.x)
            {
                i = rect->max.x;
            }
            if(j > rect->max.y)
            {
                j = rect->max.y;
            }
            //printf("locations %f %f\n", i, j);
            ARRAY_PUSH(memory_index, sh->gm, arr, SpacialHashPointToBucket(sh, v2{i, j}));
        }
    }

    return arr;
}

/* Determine which bucket to put the rect*/
SpatialStore **GetListOfPossibleBuckets(SpacialHash *sh, Rect *rect)
{
    ARRAY_CREATE(SpatialStore *, sh->gm, arr);

    //for(memory_index i = 0; i < sh->bucketCount; i++)
    //{
    //    ARRAY_PUSH(SpatialStore *, sh->gm, arr, &sh->buckets[i]);
    //}

    return arr;
}

/*It's possible that you may need to insert the rect to multiple buckets*/
void SpacialHashInsert(SpacialHash *sh, Rect *rect)
{
    ASSERT(sh->cellGrid > 0);
    for(f32 i = rect->min.x; i < rect->max.x + sh->cellGrid; i += sh->cellGrid)
    {
        for(f32 j = rect->min.y; j < rect->max.y + sh->cellGrid; j += sh->cellGrid)
        {
            if(i > rect->max.x)
            {
                i = rect->max.x;
            }
            if(j > rect->max.y)
            {
                j = rect->max.y;
            }
            printf("locations %f %f\n", i, j);
            Rect **rects = sh->buckets[SpacialHashPointToBucket(sh, v2{i, j})].rects;
            if(rects == nullptr)
            {
                ARRAY_CREATE(Rect *, sh->gm, arr);
                rects = arr;
            }

            ARRAY_PUSH(Rect *, sh->gm, rects, rect);
            //if(sh->buckets[SpacialHashPointToBucket(sh, v2{i, j})].rects != 0)
            //{
            //    printf("collision detected\n");
            //}
            //sh->buckets[SpacialHashPointToBucket(sh, v2{i, j})].rects = rect;
            printf("buckets->rect size: %zu\n", ARRAY_LIST_SIZE(rects));
            sh->buckets[SpacialHashPointToBucket(sh, v2{i, j})].rects = rects;

        }
    }
}

/*Get the bucket/list of rects that correspond to the rect bounding box */
Rect **SpacialHashGet(SpacialHash *sh, Rect *rect)
{
    ASSERT(sh->cellGrid > 0);
    /* This should be a hash set */
    //ARRAY_CREATE(Rect *, sh->gm, arr);

    HashSet *hs = CreateHashSet(sh->gm);
    for(f32 i = rect->min.x; i < rect->max.x + sh->cellGrid; i += sh->cellGrid)
    {
        for(f32 j = rect->min.y; j < rect->max.y + sh->cellGrid; j += sh->cellGrid)
        {
            if(i > rect->max.x)
            {
                i = rect->max.x;
            }
            if(j > rect->max.y)
            {
                j = rect->max.y;
            }
            if(sh->buckets[SpacialHashPointToBucket(sh, v2{i, j})].rects != 0)
            {
                /* This should be a spacial hash set */
                //ARRAY_PUSH(Rect *, sh->gm, arr, sh->buckets[SpacialHashPointToBucket(sh, v2{i, j})].rects);
                printf("%p\n",sh->buckets[SpacialHashPointToBucket(sh, v2{i, j})].rects);
                Rect **rects = sh->buckets[SpacialHashPointToBucket(sh, v2{i, j})].rects;
                for(memory_index i = 0; i < ARRAY_LIST_SIZE(rects); i++)
                {
                    HashSetInsert(sh->gm, hs, rects[i]);
                }

                //ARRAY_FREE(rects); /* to be implemented */
            }
        }
    }

    Rect **rectList = HashSetOrderedToList(sh->gm, hs);
    return rectList;
    //return arr;
}

#endif
