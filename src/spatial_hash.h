#ifndef __SPATIAL_HASH_H__
#define __SPATIAL_HASH_H__

#include "game_memory.h"
#include "array.h"
#include "hash_set.h"

struct SpatialStore
{
    Rect **rects;
};

struct SpatialHash
{
    SpatialStore *buckets;
    memory_index bucketCount;
    memory_index cellGrid;
    memory_index bucketsPerColumn;
    struct GameMemory *gm;
};
#endif
