#ifndef __SPATIAL_HASH__
#define __SPATIAL_HASH__

#include "spatial_hash.h"

SpatialHash *CreateSpatialHash(
        GameMemory *gm,
        memory_index bucketCount,
        memory_index bucketsPerColumn,
        memory_index cellGrid)
{
    SpatialHash *result = (SpatialHash *)AllocateMemory0(gm, sizeof(SpatialHash));
    result->buckets = (SpatialStore *)AllocateMemory0(gm, sizeof(SpatialStore) * bucketCount);
    result->cellGrid = cellGrid;
    result->bucketsPerColumn = bucketsPerColumn;
    result->bucketCount = bucketCount;
    result->gm = gm;
    return result;
}


/* convert a point to a possible bucket*/
memory_index SpatialHashPointToBucket(SpatialHash *sh, v2 point)
{
    /* turn buckets into a 2D grid and see if the point exist in that grid */
    /* Find an easy way to put a point into a grid */
    //if (point.x < 0)
    //    printf("x value is negative using fabsf\n");
    //if (point.y < 0)
    //    printf("y value is negative using fabsf\n");

    u32 xGrid = SafeCastToU32(fabsf(point.x) / sh->cellGrid);
    u32 yGrid = SafeCastToU32(fabsf(point.y) / sh->cellGrid);
    memory_index hashId = xGrid + yGrid * sh->bucketsPerColumn;

#if 0
    printf("float x %f\n", point.x);
    printf("float y %f\n", point.y);
    printf("x %d\n", xGrid);
    printf("y %d\n", yGrid);
    printf("hashId %zu\n", hashId);
#endif

    return hashId;
}

memory_index *SpatialHashRectToKey(SpatialHash *sh, Rect *rect)
{

    ARRAY_CREATE(memory_index, sh->gm, arr);

#if 0
    v2 bottomLeft  = rect->min;
    v2 bottomRight  = v2{rect->max.x, rect->min.y};
    v2 topLeft = v2{rect->min.x, rect->max.y};
    v2 topRight = rect->max;

    ARRAY_PUSH(memory_index, sh->gm, arr, SpatialHashPointToBucket(sh, topRight));
    ARRAY_PUSH(memory_index, sh->gm, arr, SpatialHashPointToBucket(sh, bottomLeft));
    ARRAY_PUSH(memory_index, sh->gm, arr, SpatialHashPointToBucket(sh, topLeft));
    ARRAY_PUSH(memory_index, sh->gm, arr, SpatialHashPointToBucket(sh, bottomRight));
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
            ARRAY_PUSH(memory_index, sh->gm, arr, SpatialHashPointToBucket(sh, v2{i, j}));
        }
    }

    return arr;
}

/* Determine which bucket to put the rect*/
SpatialStore **GetListOfPossibleBuckets(SpatialHash *sh, Rect *rect)
{
    ARRAY_CREATE(SpatialStore *, sh->gm, arr);

    //for(memory_index i = 0; i < sh->bucketCount; i++)
    //{
    //    ARRAY_PUSH(SpatialStore *, sh->gm, arr, &sh->buckets[i]);
    //}

    return arr;
}

/*It's possible that you may need to insert the rect to multiple buckets*/
void SpatialHashInsert(SpatialHash *sh, Rect *rect)
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
            //printf("locations %f %f\n", i, j);
            memory_index bucketId = SpatialHashPointToBucket(sh, v2{i, j});
            Rect **rects = sh->buckets[bucketId].rects;
            if(rects == nullptr)
            {
                ARRAY_CREATE(Rect *, sh->gm, arr);
                rects = arr;
            }

            ARRAY_PUSH(Rect *, sh->gm, rects, rect);
            //if(sh->buckets[SpatialHashPointToBucket(sh, v2{i, j})].rects != 0)
            //{
            //    printf("collision detected\n");
            //}
            //sh->buckets[SpatialHashPointToBucket(sh, v2{i, j})].rects = rect;
            //printf("buckets->rect size: %zu\n", ARRAY_LIST_SIZE(rects));
            sh->buckets[bucketId].rects = rects;

        }
    }
}

/*Get the bucket/list of rects that correspond to the rect bounding box */
Rect **SpatialHashGet(SpatialHash *sh, Rect *rect)
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
            memory_index bucketId = SpatialHashPointToBucket(sh, v2{i, j});
            ASSERT(bucketId < sh->bucketCount);
            if(sh->buckets[bucketId].rects != 0)
            {
                /* This should be a Spatial hash set */
                //ARRAY_PUSH(Rect *, sh->gm, arr, sh->buckets[SpatialHashPointToBucket(sh, v2{i, j})].rects);
                //printf("%p\n",sh->buckets[SpatialHashPointToBucket(sh, v2{i, j})].rects);
                Rect **rects = sh->buckets[bucketId].rects;
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

Rect **SpatialHashGet(SpatialHash *sh, AABB *range)
{
    v2 dim = 2 * range->halfDim;
    v3 center = V3(range->center, 0);

    Rect tempRect = {};
    tempRect.center = center;

    v2 centerXY = {0.5f * dim.x, 0.5f * dim.y};
    v2 basePositionXY = { center.x, center.y };
    v2 min = basePositionXY - centerXY;
    v2 max = basePositionXY + centerXY;
    tempRect.min = min;
    tempRect.max = max;
    tempRect.dim = dim;

    return SpatialHashGet(sh, &tempRect);

}

#endif
