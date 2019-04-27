#ifndef __GAME_MEMORY__
#define __GAME_MEMORY__
#pragma once

#include <stdlib.h>

#include "game_time.h"
#include "math.h"
#include "opengl.h"

struct GameMemory
{
    u32 maxSize;
    u32 used;
    u8 *base;
};

inline void InitializeGameMemory(GameMemory *gm, u32 size)
{
    gm->base = (u8 *)malloc(size); // NOLINT
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
    auto tentativePointer = (memory_index)(gm->used + gm->base);

    if (tentativePointer & alignmentMask)
    {
        alignOffSet = alignment - (memory_index)(tentativePointer & alignmentMask);
    }

    return alignOffSet;
}

inline void *AllocateMemory(GameMemory *gm, memory_index size)
{
    const memory_index byteAlignment = 4;
    ASSERT(size != 0); // NOLINT

    /* let's default alignment to 4 bytes. */
    memory_index padding = GetAlignmentOffSet(gm, byteAlignment);
    size += SafeCastToU32(padding);

    u8 *newAllocBase = gm->base + gm->used - 1;

    ASSERT(gm->used + size < gm->maxSize); // NOLINT
    gm->used += SafeCastToU32(size);

    return newAllocBase;
}

inline void *ReallocMemory(GameMemory *gm, void *ptr, memory_index oldSize, memory_index size)
{
    u8 *newMemory = (u8 *)AllocateMemory(gm, size);
    memset(newMemory, 0, size);
    /* TODO: There needs to be a part where we check if we can just extend
     * instead of creating a new chunk of memory
     *
     * TODO: Also need to free the previous memory, if we create a bigger memory block.
     */
    memcpy(newMemory, ptr, oldSize);

    return newMemory;
}
#endif
