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

    ParticleSystem particleSystem;

    v2 screenResolution;
    b32 initFromGameUpdateAndRender;
    struct Rect *playerRect;

    VulkanContext *vulkanContext;

    struct RectDynamicArray *rdaDebug;
    struct RectDynamicArray *rdaDebugUI;

    struct SceneManager *sm;

    glm::vec3 *objectsToBeAddedTotheWorld;
    glm::vec3 mouseDrag[2];
    v2 screenCoordinates[2];
    b32 isLeftButtonReleased;
    b32 createNewRect;
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
