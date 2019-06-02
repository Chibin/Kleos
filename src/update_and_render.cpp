#ifndef __UPDATE_AND_RENDER__
#define __UPDATE_AND_RENDER__

#include "define.h"
#include <stddef.h> /* offsetof */
#include <stdio.h>
#include <stdlib.h> /* abs */
#include <string>

#include "sdl_common.h"

#include <GL/glew.h>
#define GL3_PROTOTYPES 1

#pragma warning(push)
#pragma warning(disable : 4201)
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "logger.h"
#include "math.cpp"
#pragma warning(pop)

#include "game_memory.h"

/* cheeky way to replace malloc call for STB */
static GameMemory *g_reservedMemory = nullptr;
inline void *RequestToReservedMemory(memory_index size)
{
    ASSERT(g_reservedMemory);
    return AllocateMemory(g_reservedMemory, size);
}

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_MALLOC(sz) RequestToReservedMemory(sz)
/* We also need to take care of STBI_FREE(p) */
#pragma warning(push)
#pragma warning(disable : 4244)
#include <stb_image.h>
#pragma warning(pop)
#endif

#include "bitmap.h"
#include "game_metadata.h"

/* probably need to be here? depends where we put our game logic */
#include "bitmap.cpp"
#include "camera.cpp"
#include "entity.cpp"
#include "frame.cpp"
#include "entity_manager.cpp"
#include "font.cpp"
#include "game_time.cpp"
#include "shaders.cpp"

#include "collision.cpp"
#include "rect_manager.cpp"

#include "rectangle.cpp"

#include "particle.cpp"
#include "render_group.h"
#include "asset.cpp"
#include "sort.cpp"
#include "string.h"

#include "movement.cpp"
#include "hashtable.cpp"
#include "hash.h"

#include "input.cpp"

#include "renderer/vulkan/my_vulkan.cpp"
#include "renderer/common.cpp"

#include "debug_helper.cpp"
#include "random.cpp"
#include "scene_node.cpp"
#include "npc.cpp"

#define UPDATEANDRENDER(name) \
    bool name(GameMetadata *gameMetadata)

void Render(GameMetadata *gameMetadata, VulkanContext *vc);
void Update(GameMetadata *gameMetadata);
void LoadStuff(GameMetadata *gameMetadata);
inline void LoadAssets(GameMetadata *gameMetadata);

/* TODO: We'll need to get rid of these global variables later on */
EntityManager *g_entityManager = nullptr;
RectManager *g_rectManager = nullptr;
EntityDynamicArray *g_eda = nullptr;
v3 g_mousePoint;
static bool g_debugMode = false;
static VulkanBuffers g_vkBuffers;
static memory_index g_bitmapID = 0;
static memory_index g_entityID = 0;

#include "render_helper.cpp"
#include "update.cpp"
#include "render.cpp"

extern "C" UPDATEANDRENDER(UpdateAndRender)
{
    b32 continueRunning = true;
    VulkanContext *vc = gameMetadata->vulkanContext;

    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;
    ClearMemoryUsed(perFrameMemory);

#if 0
    DoVulkanDepthStencil(vc);
#endif
    DoVukanUpdateToNextFrame(vc);

    if (!gameMetadata->initFromGameUpdateAndRender)
    {
        InitGameUpdateAndRender(vc, gameMetadata);
    }

    HandleInput(gameMetadata, &continueRunning);

    SetPerFrameData(gameMetadata, perFrameMemory, gameMetadata->camera, gameMetadata->projection);

    Update(gameMetadata);
    Render(gameMetadata, vc);

    return continueRunning;
}
#endif
