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

#ifndef WIN32
/* need to figure out where to put this or do something else */
#define sprintf_s(buffer, buffer_size, stringbuffer, ...) (sprintf(buffer, stringbuffer, __VA_ARGS__))
#endif

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

void Render(GameMetadata *gameMetadata, RectDynamicArray *hitBoxes, RectDynamicArray *hurtBoxes,
            RenderGroup *perFrameRenderGroup, VulkanContext *vc);
void Update(GameMetadata *gameMetadata, GameTimestep *gameTimestep, RectDynamicArray *hitBoxes,
            RectDynamicArray *hurtBoxes, RenderGroup *perFrameRenderGroup);
void LoadStuff(GameMetadata *gameMetadata);
inline void LoadAssets(GameMetadata *gameMetadata);

/* TODO: We'll need to get rid of these global variables later on */
Camera *g_camera = nullptr;
glm::mat4 *g_projection = nullptr;
EntityManager *g_entityManager = nullptr;
Entity *g_player = nullptr;
RectManager *g_rectManager = nullptr;
EntityDynamicArray *g_eda = nullptr;
v3 g_mousePoint;
static bool g_debugMode = false;
static bool g_spriteDirectionToggle = false;
static Animation2D *g_spriteAnimation = nullptr;
static Animation2D *g_spriteAnimations = nullptr;
static VulkanBuffers g_vkBuffers;
static memory_index g_bitmapID = 0;
static NPC *g_enemyNPC = nullptr;
static SceneManager *g_sceneManager = nullptr;

#include "render_helper.cpp"
#include "update.cpp"
#include "render.cpp"

extern "C" UPDATEANDRENDER(UpdateAndRender)
{
    bool continueRunning = true;
    v2 screenResolution = gameMetadata->screenResolution;
    GameTimestep **gameTimestep = &gameMetadata->gameTimestep;
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;
    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;

    ClearMemoryUsed(perFrameMemory);

    VulkanContext *vc = gameMetadata->vulkanContext;

#if 0
    if (vc->depthStencil > 0.99f)
    {
        vc->depthIncrement = -0.001f;
    }
    if (vc->depthStencil < 0.8f)
    {
        vc->depthIncrement = 0.001f;
    }

    vc->depthStencil += vc->depthIncrement;
#endif
    //Wait for work to finish before updating MVP.
    vkDeviceWaitIdle(vc->device);
    vc->curFrame++;
    if (vc->frameCount != INT32_MAX && vc->curFrame == vc->frameCount)
    {
        vc->quit = true;
    }

    if (!gameMetadata->initFromGameUpdateAndRender)
    {
        START_DEBUG_TIMING();

        stbi_set_flip_vertically_on_load(1);

        ASSERT(!g_reservedMemory);
        ASSERT(!g_rectManager);
        ASSERT(!*gameTimestep);
        ASSERT(!g_camera);
        ASSERT(!g_eda);
        ASSERT(!g_projection);
        ASSERT(!g_entityManager);

        g_reservedMemory = reservedMemory;

        /* TODO: May be the entity manager should be the only one creating the
         * entities?
         */
        g_entityManager = CreateEntityManger(reservedMemory);
        g_eda = CreateEntityDynamicArray(reservedMemory);
        g_rectManager = CreateRectManager(reservedMemory);
        gameMetadata->rectManager = g_rectManager;

        LoadAssets(gameMetadata);

        ARRAY_CREATE(glm::vec3, &gameMetadata->reservedMemory, worldArr);
        gameMetadata->objectsToBeAddedTotheWorld = worldArr;

        SetFont(gameMetadata);
        SetHash(gameMetadata);

        g_vkBuffers.count = 0;
        g_vkBuffers.maxNum = 100;

        /* Creating pipeline layout, descriptor pool, and render pass can be done
         * indenpendently
         */
        VulkanPrepareDescriptorPool(vc);

        vc->pipelineLayout = {};
        VulkanInitPipelineLayout(vc);

        Bitmap stringBitmap = {};
        SetBitmapToGPUForPipeline(vc, gameMetadata, &stringBitmap);

        /* XXX: This is needed so that we can bind a descriptor set to a pipeline.
         * There might be a better way of doing this. This is just a hack.*/
        vc->descSet = &vc->vdsi[0].descSet;
        SetVulkanDescriptorSet(vc, gameMetadata, &stringBitmap);

        VulkanInitRenderPass(vc);
        VulkanInitFrameBuffers(vc);
        SetPreparePipeline(vc);

        SetGameTimeStep(gameMetadata);
        SetCamera(gameMetadata);
        SetPerspectiveProjection(gameMetadata);
        SetPlayer(gameMetadata);
        SetParticle(gameMetadata);

        LoadStuff(gameMetadata);

        gameMetadata->initFromGameUpdateAndRender = true;
        END_DEBUG_TIMING();
    }

    HandleInput(gameMetadata, &continueRunning);

    RectDynamicArray *hitBoxes = CreateRectDynamicArray(perFrameMemory, 100);
    RectDynamicArray *hurtBoxes = CreateRectDynamicArray(perFrameMemory, 100);

    const u32 numOfPointsPerRect = 6;
    const u16 maxEntityCount = 20001;
    RenderGroup perFrameRenderGroup = {};
    u32 vertexBlockSize = sizeof(Vertex) * numOfPointsPerRect * maxEntityCount;
    perFrameRenderGroup.vertexMemory.base = (u8 *)AllocateMemory(perFrameMemory, vertexBlockSize);
    perFrameRenderGroup.vertexMemory.maxSize = vertexBlockSize;

    u32 rectMemoryBlockSize = sizeof(Rect) * maxEntityCount;
    perFrameRenderGroup.rectMemory.base = (u8 *)AllocateMemory(perFrameMemory, rectMemoryBlockSize);
    perFrameRenderGroup.rectMemory.maxSize = rectMemoryBlockSize;

    gameMetadata->rdaDebug = CreateRectDynamicArray(perFrameMemory, 10000);
    gameMetadata->rdaDebugUI = CreateRectDynamicArray(perFrameMemory);

    SceneManager *sm = (SceneManager *)AllocateMemory(perFrameMemory, sizeof(SceneManager));
    g_sceneManager = sm;
    gameMetadata->sm = sm;
    memset(sm, 0, sizeof(SceneManager));
    sm->perFrameMemory = perFrameMemory;
    sm->gameMetadata = gameMetadata;

    SetAABB(&g_rectManager->NonTraversable);
    CreateScenePartition(sm, &g_rectManager->NonTraversable);

    Update(gameMetadata, *gameTimestep, hitBoxes, hurtBoxes, &perFrameRenderGroup);
    Render(gameMetadata,
           hitBoxes,
           hurtBoxes,
           &perFrameRenderGroup,
           vc);

    return continueRunning;
}

void LoadStuff(GameMetadata *gameMetadata)
{
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;

    v4 color = { 0.1f, 0.1f, 0.1f, 1.0f };
    /* load random data */
    for (int i = 0; i < 100; i++)
    {
        for (int y = 0; y < 100; y++)
        {
            v3 startingPosition = { -1 + (real32)i, 1 * (real32)y, 0 };
            /* TODO: extract out creating new entity from the manager */
            Entity *rectEntity =
                AddNewEntity(reservedMemory, g_entityManager, startingPosition);
            ASSERT(rectEntity != nullptr);
            rectEntity->isTraversable = true;
            rectEntity->isPlayer = false;
            rectEntity->type = REGULAR;

            Rect *r =
                CreateRectangle(reservedMemory, startingPosition, color, 1, 1);
            AssociateEntity(r, rectEntity, true);
            r->bitmapID = FindBitmap(&gameMetadata->bitmapSentinelNode, "awesomeface")->bitmapID;
            r->renderLayer = BACKGROUND;
            PushBack(&(g_rectManager->Traversable.rda), r);
        }
    }

    /* Spawn new enemty */
    g_enemyNPC = (NPC *)AllocateMemory(reservedMemory, sizeof(NPC));
    memset(g_enemyNPC, 0, sizeof(NPC));

    g_enemyNPC->dim.height = 1.0f;
    g_enemyNPC->dim.width = 1.5f;
    g_enemyNPC->bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, "pshroom");
    g_enemyNPC->spriteAnimation =
        CreateCopyOfSpriteAnimationInfo(
                reservedMemory,
                GetSpriteAnimationInfo(
                    GetFrameAnimation(&gameMetadata->frameAnimationSentinelNode, "pshroom"),
                    "IDLE")
        );
    g_enemyNPC->spriteAnimation->direction = LEFT;
    g_enemyNPC->direction = LEFT;
    g_enemyNPC->renderLayer = BEHIND_PLAYER;
    g_enemyNPC->movementType = X_MOVEMENT;
    g_enemyNPC->movementPattern = UNI_DIRECTIONAL;
}

inline void LoadAssets(GameMetadata *gameMetadata)
{

    GameMemory *reservedMemory = &gameMetadata->reservedMemory;

    auto *awesomefaceBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));
    auto *newBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));
    auto *boxBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));
    auto *pshroomBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));

    SetBitmap(awesomefaceBitmap, "awesomeface", TextureParam{ GL_LINEAR, GL_LINEAR },
              g_bitmapID++, "./materials/textures/awesomeface.png");
    SetBitmap(newBitmap, "arche", TextureParam{ GL_NEAREST, GL_NEAREST },
              g_bitmapID++, "./materials/textures/arche.png");
    SetBitmap(boxBitmap, "box", TextureParam{ GL_LINEAR, GL_LINEAR },
              g_bitmapID++, "./materials/textures/container.png");
    SetBitmap(pshroomBitmap, "pshroom", TextureParam{ GL_LINEAR, GL_LINEAR },
              g_bitmapID++, "./materials/textures/pshroom.png");

    PushBitmap(&gameMetadata->bitmapSentinelNode, awesomefaceBitmap);
    PushBitmap(&gameMetadata->bitmapSentinelNode, newBitmap);
    PushBitmap(&gameMetadata->bitmapSentinelNode, boxBitmap);
    PushBitmap(&gameMetadata->bitmapSentinelNode, pshroomBitmap);

    PushBitmap(&gameMetadata->bitmapSentinelNode, &gameMetadata->whiteBitmap);
    PushBitmap(&gameMetadata->bitmapSentinelNode, &gameMetadata->fontBitmap);

    Bitmap *archeBitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, newBitmap->bitmapID);
    ASSERT(archeBitmap != nullptr);

    /* XXX: There's a loose coupling between frame animation and bitmap width
     * and height. We need the width and height to make a conversion to pixel
     * to UV coordinates
     */
    u32 bitmapWidth = archeBitmap->width;
    u32 bitmapHeight = archeBitmap->height;

    LoadFrameData(gameMetadata, "./assets/texture_data/frames.txt");

    FrameAnimation *fa = GetFrameAnimation(&gameMetadata->frameAnimationSentinelNode, "arche.png");
    ASSERT(fa != NULL);

    for (memory_index animCount = 0; animCount < fa->animationCount; animCount ++)
    {

        /* TODO: Have a way to find the correct animation information based on name? */
        ASSERT(fa->frameCycles[animCount].animationInfo == NULL);
        if (fa->frameCycles[animCount].animationInfo == nullptr)
        {
            fa->frameCycles[animCount].animationInfo =
                (Animation2D *)AllocateMemory(reservedMemory, sizeof(Animation2D));
            ZeroSize(fa->frameCycles[animCount].animationInfo, sizeof(Animation2D));
        }

        Animation2D *spriteAnim = fa->frameCycles[animCount].animationInfo;
        spriteAnim->direction = LEFT;
        u32 totalFrames = fa->frameCycles[animCount].frameCount;
        spriteAnim->totalFrames = totalFrames;
        spriteAnim->frameCoords =
            (RectUVCoords *)AllocateMemory(reservedMemory, sizeof(RectUVCoords) * totalFrames);
        if (strcmp(fa->frameCycles[animCount].name, "ATTACK") == 0)
        {
            spriteAnim->timePerFrame = 150;
        }
        else if (strcmp(fa->frameCycles[animCount].name, "IDLE") == 0)
        {
            spriteAnim->timePerFrame = 1000 * 0.75;
        }

        /* TODO: Need to figure out how to use compound literals with specific assignment in windows */
        v2 topRight = {}, bottomRight = {}, bottomLeft = {}, topLeft = {};
        for(memory_index i = 0; i < totalFrames; i++)
        {
            spriteAnim->frameCoords[i] =
                RectUVCoords{
                    topRight    = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[0], bitmapWidth, bitmapHeight),
                    bottomRight = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[1], bitmapWidth, bitmapHeight),
                    bottomLeft  = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[2], bitmapWidth, bitmapHeight),
                    topLeft     = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[3], bitmapWidth, bitmapHeight)
                };
        }
    }

    fa = GetFrameAnimation(&gameMetadata->frameAnimationSentinelNode, "pshroom");
    ASSERT(fa != NULL);
    Bitmap *bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, "pshroom");
    bitmapWidth = bitmap->width;
    bitmapHeight = bitmap->height;

    for (memory_index animCount = 0; animCount < fa->animationCount; animCount ++)
    {

        /* TODO: Have a way to find the correct animation information based on name? */
        ASSERT(fa->frameCycles[animCount].animationInfo == NULL);
        if (fa->frameCycles[animCount].animationInfo == nullptr)
        {
            fa->frameCycles[animCount].animationInfo =
                (Animation2D *)AllocateMemory(reservedMemory, sizeof(Animation2D));
            ZeroSize(fa->frameCycles[animCount].animationInfo, sizeof(Animation2D));
        }

        Animation2D *spriteAnim = fa->frameCycles[animCount].animationInfo;
        spriteAnim->direction = LEFT;
        u32 totalFrames = fa->frameCycles[animCount].frameCount;
        spriteAnim->totalFrames = totalFrames;
        spriteAnim->frameCoords =
            (RectUVCoords *)AllocateMemory(reservedMemory, sizeof(RectUVCoords) * totalFrames);
        if (strcmp(fa->frameCycles[animCount].name, "ATTACK") == 0)
        {
            spriteAnim->timePerFrame = 150;
        }
        else if (strcmp(fa->frameCycles[animCount].name, "IDLE") == 0)
        {
            spriteAnim->timePerFrame = 1000 * 0.75;
        }

        /* TODO: Need to figure out how to use compound literals with specific assignment in windows */
        v2 topRight = {}, bottomRight = {}, bottomLeft = {}, topLeft = {};
        for(memory_index i = 0; i < totalFrames; i++)
        {
            spriteAnim->frameCoords[i] =
                RectUVCoords{
                    topRight    = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[0], bitmapWidth, bitmapHeight),
                    bottomRight = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[1], bitmapWidth, bitmapHeight),
                    bottomLeft  = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[2], bitmapWidth, bitmapHeight),
                    topLeft     = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[3], bitmapWidth, bitmapHeight)
                };
        }
    }

    g_spriteAnimation =
        GetSpriteAnimationInfo(
                GetFrameAnimation(&gameMetadata->frameAnimationSentinelNode, "arche.png"),
                "IDLE");

    MapData *rootMDNode = LoadAssetMap("./assets/map_data/first_map.txt");
    ASSERT(rootMDNode->next != nullptr);

    for(MapData *currentNode = rootMDNode->next; currentNode != nullptr; currentNode = currentNode->next)
    {
        v4 color = { 0.1f, 0.1f, 0.1f, 1.0f };
        for (memory_index i = 0; i < currentNode->count; i++)
        {

            v3 startingPosition = V3(currentNode->basePoints[i], 0);
            Rect *collissionRect =
                CreateRectangle(reservedMemory, startingPosition, color, currentNode->dim);

            collissionRect->type = COLLISION;
            collissionRect->renderLayer = FRONT_STATIC;
            UpdateColors(collissionRect, v4{ 0.0f, 0.0f, 1.0f, 0.7f });

            Bitmap *findBoxBitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, "box");
            collissionRect->bitmapID = findBoxBitmap->bitmapID;
            PushBack(&(g_rectManager->NonTraversable.rda), collissionRect);
        }
    }
}
#endif
