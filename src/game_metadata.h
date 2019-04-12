#pragma once
#ifndef __GAME_METADATA__
#define __GAME_METADATA__

#include "bitmap.h"
#include "hashtable.h"
#include "particle.h"
#include "renderer/vulkan/vulkan.h"

#include "asset.h"

struct GameMetadata
{
    u32 totalMemoryBlockSize;
    u32 maxBlockSize;
    u32 usedBlock;
    u8 *base;

    TTF_Font *font;
    GameMemory reservedMemory;
    GameMemory temporaryMemory;
    GameTimestep *gameTimestep;

    Bitmap bitmapSentinelNode;
    Bitmap whiteBitmap;

    BitmapDescriptorMap bitmapToDescriptorSetMap;

    FrameAnimation frameAnimationSentinelNode;

    GLuint vaoID;
    GLuint eboID;
    GLuint vboID;
    GLuint textureID;

    GLuint program;
    GLuint debugProgram;

    ParticleSystem particleSystem;

    v2 screenResolution;
    b32 initFromGameUpdateAndRender;
    struct Rect *playerRect;

    VulkanContext *vulkanContext;
    b32 isVulkanActive;
    b32 isOpenGLActive;
    b32 initOpenGL;

    struct RectDynamicArray *rdaDebug;

    struct SceneManager *sm;
};

inline void *AllocateMemory(GameMetadata *gm, u32 size)
{
    ASSERT(size != 0);
    ASSERT(gm->usedBlock + size <= gm->maxBlockSize);
    u8 *newAllocBase = gm->base + gm->usedBlock - 1;
    gm->usedBlock += size;
    return newAllocBase;
}
#endif
