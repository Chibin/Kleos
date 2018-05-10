#pragma once
#include "math.h"
#include "sdl_common.h"

#ifndef __GAME_MEMORY__
#define __GAME_MEMORY__

#include "opengl.h"

enum ImageType
{
    STB = 0x1,
    SDL = 0x2,
};

struct Bitmap
{
    u32 width;
    u32 height;
    GLenum format;
    u8* data;
    ImageType freeImageType;

    TextureParam textureParam;
    memory_index bitmapID;

    Bitmap *next;
    Bitmap *prev;
};

struct GameMemory
{
    u32 maxSize;
    u32 used;
    u8 *base;
};

struct GameTimestep;
struct GameMetadata
{
    u32 totalMemoryBlockSize;
    u32 maxBlockSize;
    u32 usedBlock;
    u8 *base;

    TTF_Font *font;
    Bitmap *bitmaps[3];
    GameMemory reservedMemory;
    GameMemory transientMemory;
    GameTimestep *gameTimestep;
    Bitmap whiteBitmap;
    Bitmap sentinelNode;
};

inline void PushBitmap(Bitmap *oldNode, Bitmap *newNode)
{
    newNode->next = oldNode->next;
    newNode->prev = oldNode;
    oldNode->next->prev = newNode;
    oldNode->next = newNode;
}

inline Bitmap *FindBitmap(Bitmap *sentinelNode, memory_index bitmapID)
{
    for(Bitmap * node = sentinelNode->next; node != sentinelNode; node->next)
    {
        if (node->bitmapID == bitmapID)
        {
            return node;
        }
    }

    return nullptr;
}

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

inline void *AllocateMemory(GameMemory *gm, memory_index size, const memory_index byteAlignment = 4)
{
    ASSERT(size != 0);

    /* let's default alignment to 4 bytes. */
    memory_index padding = GetAlignmentOffSet(gm, byteAlignment);
    size += SafeCastToU32(padding);

    u8* newAllocBase = gm->base + gm->used - 1;

    ASSERT(gm->used + size < gm->maxSize);
    gm->used += SafeCastToU32(size);

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
