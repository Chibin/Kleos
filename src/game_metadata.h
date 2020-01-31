#pragma once
#ifndef __GAME_METADATA__
#define __GAME_METADATA__

#include "bitmap.h"
#include "hashtable.h"
#include "particle.h"
#include "renderer/vulkan/vulkan.h"

#include "asset.h"
#include "rect_manager.h"
#include "hash.h"
#include "mouse.h"
#include "spatial_hash.h"

struct EditMode
{
    b32 isActive;
    b32 isCommandPrompt;

    char commandPrompt[256];
    char backupCommandPrompt[256];
    memory_index commandPromptCount;

    Rect *selectedRect;

    glm::vec3 *objectsToBeAddedTotheWorld;
    glm::vec3 leftMouseDrag[2];
    b32 createNewRect;

    v3 selectUITextureOffset;
};

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
    Bitmap fontBitmap;

    Hash *hashBitmapVkDescriptorSet;
    Hash *hashBitmap;
    Hash *hashEntityRect;
    Hash *hashEntityMovement;
    Hash *hashEntityNPC;
    Hash *hashSetEntity;
    Hash *hashCharUIInfo;
    Hash *hashEntityStat;
    memory_index entityID;

    FrameAnimation frameAnimationSentinelNode;

    ParticleSystem particleSystem;

    v2 screenResolution;
    b32 initFromGameUpdateAndRender;
    struct Entity *playerEntity;

    struct VulkanContext *vulkanContext;

    struct RectDynamicArray *rdaDebug;
    struct RectDynamicArray *rdaDebugUI;
    struct RectDynamicArray *hurtBoxes;
    struct RectDynamicArray *hitBoxes;

    RenderGroup *perFrameRenderGroup;
    RenderGroup *perFrameRenderGroupUI;

    struct SceneManager *sm;

    struct SpatialHash *sh;

    EditMode editMode;

    struct RectManager *rectManager;

    glm::vec3 infinitePlaneNormal;

    struct Camera *camera;
    glm::mat4 *projection;

    MouseInfo mouseInfo;
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
