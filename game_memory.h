#pragma once
#include "math.h"
#include "sdl_common.h"

#ifndef __GAME_MEMORY__
#define __GAME_MEMORY__

#define KILOBYTE(x) 1024*x
#define MEGABYTE(x) KILOBYTE(1024)*x
#define GIGABYTE(x) MEGABYTE(1024)*x

struct GameMemory {
    u32 maxSize;
    u32 used;
    u8 *base;
};

inline void ZeroSize(void *_data, memory_index size)
{
    u8 *base = (u8 *)_data;
    while(size--)
    { 
        *(base + size) = 0;
    }
}

inline void InitializeGameMemory(GameMemory *gm, u32 size)
{
    gm->base = (u8 *)malloc(size);
    gm->maxSize = size;
    gm->used = 0;
}

inline void *AllocateMemory(GameMemory *gm, u32 size)
{
    ASSERT(size != 0);
    ASSERT(gm->used + size < gm->maxSize);
    u8* newAllocBase = gm->base + gm->used - 1;
    gm->used += size;
    return newAllocBase;
}

struct GameTimestep;
struct GameMetadata {
    u32 totalMemoryBlockSize;
    u32 maxBlockSize;
    u32 usedBlock;
    u8 *base;

    GameMemory reservedMemory;
    GameMemory transientMemory;
    GameTimestep *gameTimestep;
};

inline void *AllocateMemory(GameMetadata *gm, u32 size)
{
    ASSERT(size != 0);
    ASSERT(gm->usedBlock + size <= gm->maxBlockSize);
    u8* newAllocBase = gm->base + gm->usedBlock - 1;
    gm->usedBlock += size;
    return newAllocBase;
}
#endif
