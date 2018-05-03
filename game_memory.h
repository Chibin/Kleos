#pragma once
#include "math.h"
#include "sdl_common.h"

#ifndef __GAME_MEMORY__
#define __GAME_MEMORY__

struct GameMemory {
    u32 maxSize;
    u32 used;
    u8 *base;
};

struct GameTimestep;
struct GameMetadata {
    u32 totalMemoryBlockSize;
    u32 maxBlockSize;
    u32 usedBlock;
    u8 *base;

    TTF_Font *font;
    struct Bitmap *bitmaps[3];
    GameMemory reservedMemory;
    GameMemory transientMemory;
    GameTimestep *gameTimestep;
};

inline void InitializeGameMemory(GameMemory *gm, u32 size)
{
    gm->base = (u8 *)malloc(size);
    gm->maxSize = size;
    gm->used = 0;
}

inline void ClearMemoryUsed(GameMemory *gm)
{
    gm->used = 0;
}

inline memory_index GetAlignmentOffSet(GameMemory *gm, memory_index alignment)
{
    memory_index alignOffSet = 0;

    memory_index alignmentMask = alignment - 1;
    memory_index tentativePointer = (memory_index)(gm->used + gm->base);

    if (tentativePointer & alignmentMask) {
        alignOffSet = alignment - (memory_index)(tentativePointer & alignmentMask);
    }

    return alignOffSet;
}

inline void *AllocateMemory(GameMemory *gm, u32 size, const memory_index byteAlignment = 4)
{
    ASSERT(size != 0);

    /* let's default alignment to 4 bytes. */
    memory_index padding = GetAlignmentOffSet(gm, byteAlignment);
    size += SafeCastToU32(padding);

    u8* newAllocBase = gm->base + gm->used - 1;

    ASSERT(gm->used + size < gm->maxSize);
    gm->used += size;

    return newAllocBase;
}

inline void *AllocateMemory(GameMetadata *gm, u32 size)
{
    ASSERT(size != 0);
    ASSERT(gm->usedBlock + size <= gm->maxBlockSize);
    u8* newAllocBase = gm->base + gm->usedBlock - 1;
    gm->usedBlock += size;
    return newAllocBase;
}
#endif
