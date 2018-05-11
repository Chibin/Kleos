#pragma once
#ifndef __GAME_METADATA__
#define __GAME_METADATA__

struct GameMetadata
{
    u32 totalMemoryBlockSize;
    u32 maxBlockSize;
    u32 usedBlock;
    u8 *base;

    TTF_Font *font;
    Bitmap *bitmaps[3];
    GameMemory reservedMemory;
    GameMemory temporaryMemory;
    GameTimestep *gameTimestep;
    Bitmap whiteBitmap;
    Bitmap sentinelNode;

    GLuint vaoID;
    GLuint eboID;
    GLuint vboID;
    GLuint textureID;

    GLuint program;
    GLuint debugProgram;

    v2 screenResolution;
    b32 initFromGameUpdateAndRender;
    struct Rect *playerRect;
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