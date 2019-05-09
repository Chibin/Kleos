#ifndef __RENDER__
#define __RENDER__

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
#include "input.cpp"
#include "shaders.cpp"

#include "collision.cpp"
#include "rect_manager.cpp"

#include "rectangle.cpp"

#include "particle.cpp"
#include "render_group.h"
#include "asset.cpp"
#include "sort.cpp"
#include "string.cpp"

#include "hashtable.cpp"

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

extern "C" UPDATEANDRENDER(UpdateAndRender)
{
    SDL_Event event;
    bool continueRunning = true;

    GameTimestep **gameTimestep = &gameMetadata->gameTimestep;
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;
    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;
    ClearMemoryUsed(perFrameMemory);
    v2 screenResolution = gameMetadata->screenResolution;

    VulkanContext *vc = gameMetadata->vulkanContext;

    {
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
    }

    if (!gameMetadata->initFromGameUpdateAndRender)
    {
        ARRAY_CREATE(glm::vec3, &gameMetadata->reservedMemory, worldArr);
        gameMetadata->objectsToBeAddedTotheWorld = worldArr;

        TTF_Font *font = OpenFont();
        ASSERT(font != nullptr);
        gameMetadata->font = font;
        //ASSERT(TTF_FontFaceIsFixedWidth(font) == 1);

        g_vkBuffers.count = 0;
        g_vkBuffers.maxNum = 100;

        Bitmap stringBitmap = {};

        memset(&gameMetadata->bitmapToDescriptorSetMap, 0, sizeof(BitmapDescriptorMap));
#if 1
        for (memory_index i = 0; i < MAX_HASH; i++)
        {
            ASSERT(gameMetadata->bitmapToDescriptorSetMap.hashTable[i] == nullptr);
        }
#endif

        bool useStagingBuffer = false;
        stbi_set_flip_vertically_on_load(1);

        SetWhiteBitmap(gameMetadata);
        SetFontBitmap(gameMetadata);

        s32 texWidth = 0;
        s32 texHeight = 0;
        s32 texChannels = 0;

        stringBitmap = {};
        stringBitmap.bitmapID = MAX_HASH - 2; // Arbitrary number
        char buffer[256];
        /* Hack: There's a bunch of blank spaces at the end to accomodate
         * the amout of extra characters for later images.
         * This will give us a longer width when creating a vkimage.
         */

        sprintf_s(buffer, sizeof(char) * 150, "   %.02f ms/f    %.0ff/s    %.02fcycles/f              ", 33.0f, 66.0f, 99.0f); // NOLINT
        StringToBitmap(&stringBitmap, gameMetadata->font, buffer);                                    // NOLINT
        stringBitmap.textureParam = TextureParam{ GL_NEAREST,  GL_NEAREST };

        vc->UITextures[0].texWidth = stringBitmap.width;
        vc->UITextures[0].texHeight = stringBitmap.height;
        vc->UITextures[0].texPitch = stringBitmap.pitch;
        vc->UITextures[0].dataSize = stringBitmap.size;
        vc->UITextures[0].data = stringBitmap.data;

        VulkanPrepareTexture(
                &vc->gpu,
                &vc->device,
                &vc->setupCmd,
                &vc->cmdPool,
                &vc->queue,
                &vc->memoryProperties,
                useStagingBuffer,
                vc->UITextures,
                VK_IMAGE_LAYOUT_GENERAL);

        ARRAY_CREATE(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vdsi);
        vc->vdsi = vdsi;

        VulkanDescriptorSetInfo tempVulkanDescriptorSetInfo = {};
        tempVulkanDescriptorSetInfo.textureObjCount = DEMO_TEXTURE_COUNT;
        tempVulkanDescriptorSetInfo.uniformData = &vc->uniformData;
        tempVulkanDescriptorSetInfo.uniformDataFragment = &vc->uniformDataFragment;

        tempVulkanDescriptorSetInfo.descSet = {};
        tempVulkanDescriptorSetInfo.textureObj = (TextureObject *)AllocateMemory(reservedMemory, sizeof(TextureObject));
        memset(tempVulkanDescriptorSetInfo.textureObj, 0, sizeof(TextureObject));
        tempVulkanDescriptorSetInfo.name = "awesomeface";
        tempVulkanDescriptorSetInfo.imagePath ="./materials/textures/awesomeface.png";
        ARRAY_PUSH(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vc->vdsi, tempVulkanDescriptorSetInfo);

        tempVulkanDescriptorSetInfo.descSet = {};
        tempVulkanDescriptorSetInfo.textureObj = (TextureObject *)AllocateMemory(reservedMemory, sizeof(TextureObject));
        tempVulkanDescriptorSetInfo.name = "arche";
        tempVulkanDescriptorSetInfo.imagePath = "./materials/textures/arche.png";
        ARRAY_PUSH(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vc->vdsi, tempVulkanDescriptorSetInfo);

        tempVulkanDescriptorSetInfo.descSet = {};
        tempVulkanDescriptorSetInfo.textureObj = (TextureObject *)AllocateMemory(reservedMemory, sizeof(TextureObject));
        tempVulkanDescriptorSetInfo.name = "box";
        tempVulkanDescriptorSetInfo.imagePath = "./materials/textures/container.png";
        ARRAY_PUSH(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vc->vdsi, tempVulkanDescriptorSetInfo);

        tempVulkanDescriptorSetInfo.descSet = {};
        tempVulkanDescriptorSetInfo.textureObj = (TextureObject *)AllocateMemory(reservedMemory, sizeof(TextureObject));
        tempVulkanDescriptorSetInfo.name = "pshroom";
        tempVulkanDescriptorSetInfo.imagePath = "./materials/textures/pshroom.png";
        ARRAY_PUSH(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vc->vdsi, tempVulkanDescriptorSetInfo);

        tempVulkanDescriptorSetInfo.descSet = {};
        tempVulkanDescriptorSetInfo.textureObj = (TextureObject *)AllocateMemory(reservedMemory, sizeof(TextureObject));
        tempVulkanDescriptorSetInfo.name = "white";
        tempVulkanDescriptorSetInfo.imagePath = "";
        tempVulkanDescriptorSetInfo.textureObj[0].texWidth = gameMetadata->whiteBitmap.width;
        tempVulkanDescriptorSetInfo.textureObj[0].texHeight = gameMetadata->whiteBitmap.height;
        /* stbi does not do padding, so the pitch is the component times
         * the width of the image. The component is 4 because of STBI_rgb_alpha.
         */
        tempVulkanDescriptorSetInfo.textureObj[0].texPitch = gameMetadata->whiteBitmap.width * 4;
        tempVulkanDescriptorSetInfo.textureObj[0].dataSize =
            gameMetadata->whiteBitmap.width * gameMetadata->whiteBitmap.height * 4;
        tempVulkanDescriptorSetInfo.textureObj[0].data = gameMetadata->whiteBitmap.data;
        ARRAY_PUSH(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vc->vdsi, tempVulkanDescriptorSetInfo);

        tempVulkanDescriptorSetInfo.descSet = {};
        tempVulkanDescriptorSetInfo.textureObj = (TextureObject *)AllocateMemory(reservedMemory, sizeof(TextureObject));
        tempVulkanDescriptorSetInfo.name = "font";
        tempVulkanDescriptorSetInfo.imagePath = "";
        tempVulkanDescriptorSetInfo.textureObj[0].texWidth = gameMetadata->fontBitmap.width;
        tempVulkanDescriptorSetInfo.textureObj[0].texHeight = gameMetadata->fontBitmap.height;
        /* stbi does not do padding, so the pitch is the component times
         * the width of the image. The component is 4 because of STBI_rgb_alpha.
         */
        tempVulkanDescriptorSetInfo.textureObj[0].texPitch = gameMetadata->fontBitmap.width * 4;
        tempVulkanDescriptorSetInfo.textureObj[0].dataSize =
            gameMetadata->fontBitmap.width * gameMetadata->fontBitmap.height * 4;
        tempVulkanDescriptorSetInfo.textureObj[0].data = gameMetadata->fontBitmap.data;
        ARRAY_PUSH(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vc->vdsi, tempVulkanDescriptorSetInfo);

        for(memory_index i = 0; i < ARRAY_LIST_SIZE(vc->vdsi); i++)
        {
            VulkanLoadImageToGPU(vc, &vc->vdsi[i]);
        }

        /* XXX: This is needed so that we can bind a descriptor set to a pipeline.
         * There might be a better way of doing this. This is just a hack.*/
        vc->descSet = &vc->vdsi[0].descSet;

        /* Creating pipeline layout, descriptor pool, and render pass can be done
         * indenpendently
         */
        VulkanPrepareDescriptorPool(vc);

        vc->pipelineLayout = {};
        VulkanInitPipelineLayout(vc);

        for (memory_index i = 0; i < ARRAY_LIST_SIZE(vc->vdsi); i++)
        {
            VulkanSetDescriptorSet(vc, &vc->vdsi[i]);
        }

        VulkanSetDescriptorSet(
                vc,
                &vc->secondDescSet,
                vc->UITextures,
                DEMO_TEXTURE_COUNT,
                &vc->uniformData,
                &vc->uniformDataFragment);

        VulkanInitRenderPass(vc);
        VulkanInitFrameBuffers(vc);

        memset(&vc->vertices, 0, sizeof(vc->vertices));
        VulkanPreparePipeline(vc, sizeof(Vertex));
        VulkanPrepare2ndPipeline(vc, sizeof(Vertex));

        START_DEBUG_TIMING();

        ASSERT(!*gameTimestep);
        ASSERT(!g_camera);
        ASSERT(!g_eda);
        ASSERT(!g_projection);
        ASSERT(!g_rectManager);
        ASSERT(!g_entityManager);
        ASSERT(!g_reservedMemory);

        g_reservedMemory = reservedMemory;

        *gameTimestep = (GameTimestep *)AllocateMemory(reservedMemory, sizeof(GameTimestep));
        ResetGameTimestep(*gameTimestep);

        v3 cameraPos = { 0, 0, 5 };
        // and looks at the origin
        v3 cameraTarget = { 0, 0, 0 };
        // Head is up (set to 0,1,0 to look upside-down)
        v3 cameraUp = { 0, 1, 0 };
        g_camera = CreateCamera(reservedMemory, cameraPos, cameraTarget, cameraUp);

        g_eda = CreateEntityDynamicArray(reservedMemory);

        f32 screenWidth = screenResolution.v[0];
        f32 screenHeight = screenResolution.v[1];
        g_projection = (glm::mat4 *)AllocateMemory(reservedMemory, (sizeof(glm::mat4)));
        *g_projection =
            //glm::perspective(glm::radians(45.0f), screenWidth / screenHeight, 0.1f, 1000.0f);
            //glm::infinitePerspective(45.0f, screenWidth / screenHeight, 1.0f);
            glm::mat4(PerspectiveProjectionMatrix(Radians(45.0f), screenWidth / screenHeight, 0.1f, 1000.0f));

        g_rectManager = CreateRectManager(reservedMemory);

        /* TODO: May be the entity manager should be the only one creating the
         * entities?
         */
        g_entityManager = CreateEntityManger(reservedMemory);

        LoadAssets(gameMetadata);
        for(memory_index i = 0; i < ARRAY_LIST_SIZE(vc->vdsi); i++)
        {
            HashInsert(
                    &gameMetadata->bitmapToDescriptorSetMap,
                    FindBitmap(&gameMetadata->bitmapSentinelNode, vc->vdsi[i].name),
                    &vc->vdsi[i].descSet);
        }

        HashInsert(
                &gameMetadata->bitmapToDescriptorSetMap,
                &stringBitmap,
                &vc->secondDescSet);

        Entity newPlayer = {};
        newPlayer.position = glm::vec3(0, 0, 0);
        newPlayer.isPlayer = true;

        int index = Append(reservedMemory, g_entityManager, &newPlayer);

        /* NOTE: we don't need to free the player since we created it in the
         * stack
         */
        v3 pos = { 0, 0, 0.01f };
        g_player = &g_entityManager->entities[index];
        g_player->type = 2;
        gameMetadata->playerRect = CreateRectangle(reservedMemory, pos, COLOR_WHITE, 2, 1);
        AssociateEntity(gameMetadata->playerRect, g_player, false);
        g_rectManager->player = gameMetadata->playerRect;
        gameMetadata->playerRect->type = REGULAR;
        gameMetadata->playerRect->renderLayer = PLAYER;
        gameMetadata->playerRect->frameDirection = LEFT;
        gameMetadata->playerRect->bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, "arche");
        gameMetadata->playerRect->bitmapID = gameMetadata->playerRect->bitmap->bitmapID;

        LoadStuff(gameMetadata);

        gameMetadata->initFromGameUpdateAndRender = true;

        //ParticleSystem *ps = &gameMetadata->particleSystem;
        f32 base = -1.01f;
        f32 width = 0.05f;
        f32 height = 0.05f;
#if 1
        Bitmap *bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, (memory_index)0);
#else
        Bitmap *bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, "box");
#endif
        gameMetadata->particleSystem.particleCount = 10;
        gameMetadata->particleSystem.particles =
            (Particle *)AllocateMemory(reservedMemory,
                                       gameMetadata->particleSystem.particleCount * sizeof(Particle));

        for (memory_index i = 0; i < gameMetadata->particleSystem.particleCount; i++)
        {
            Particle *particle = &gameMetadata->particleSystem.particles[i];
            ZeroSize(particle, sizeof(Particle));
            particle->TTL = particle->maxTTL = 60000;

            v3 basePosition = { base, base, 0.5f };
            base += 0.15f;
            v4 color = { 0.1f, 0.1f, 0.1f, 1 };
            particle->rect.color = color;
            particle->rect.bitmapID = 0;
            particle->rect.bitmap = bitmap;
            particle->rect.renderLayer = BEHIND_PLAYER;
            particle->positionOffset = basePosition;
            SetRectPoints(&particle->rect, basePosition, width, height);
        }

        END_DEBUG_TIMING();
    }

    /* NOTE: Looks very player centric right now, not sure if we need to make it
     * better later on.
     * Use keystate to get the feel for the fastest response time.
     * Keyrepeat is doesn't feel smooth enough for us to use it as a way to
     * move. The amount of keyboard repeat depends on the setting the user has
     * on their computer, so not reliable.
     */
    const u8 *keystate = SDL_GetKeyboardState(nullptr);
    ProcessKeysHeldDown(g_player, keystate);

    while (SDL_PollEvent(&event) != 0)
    {
        switch (event.type)
        {
        case SDL_QUIT:
            return false;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            ProcessMouseButton(event.button);
            AddNewRectToWorld(gameMetadata, g_camera, g_projection, event);
        case SDL_MOUSEWHEEL:
            ProcessMouseInput(event, g_camera);
            break;
        case SDL_MOUSEMOTION:
            g_mousePoint = ProcessMouseMotion(event.motion);
            UpdateMouseDrag(gameMetadata, g_camera, g_projection, event);
            break;
        case SDL_KEYDOWN:
            ProcessInputDown(
                    event.key.keysym.sym,
                    gameMetadata,
                    &continueRunning);
            break;
        case SDL_KEYUP:
            ProcessInputUp(event.key.keysym.sym);
            break;
        default:
            break;
        }
    }

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

void UpdateEntities(GameMetadata *gameMetadata, GameTimestep *gt, RectDynamicArray *hitBoxes, RectDynamicArray *hurtBoxes, RenderGroup *perFrameRenderGroup, bool isPlayer = false)
{
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;

    /*
     * Attack animation startup
     * Create animation hitboxes
     * Create animation hurtboxes
     * Update movement location
     */

    /* NOTE: Look at this later Axis-Aligned Bounding Box*/
    // position, velocity, acceleration
    const GLfloat gravity = -9.81f;

    f32 dt = gt->dt;

    Entity *e = GetEntity(gameMetadata->playerRect);

    //      if (IntersectionAABB(rect, v2{e->position.x, e->position.y},
    //      rayDirection))
    v2 center = 0.5f * v2{gameMetadata->playerRect->width, gameMetadata->playerRect->height};
    Rect nextUpdate = *gameMetadata->playerRect;
    nextUpdate.min = V2(e->position) + V2(e->velocity) * dt + 0.5f * V2(e->acceleration) * dt * dt - center;
    nextUpdate.max = V2(e->position) + V2(e->velocity) * dt + 0.5f * V2(e->acceleration) * dt * dt + center;

    /* XXX: If we're on the ground we should always reset the accelration to the force of gravity. */
    e->acceleration.y += gravity;

    MinMax temp = GetMinMax(&nextUpdate);
    AABB range = MinMaxToSquareAABB(&temp);
#if 1
    f32 arbitraryPadding = 5.0f;
#else
    f32 arbitraryPadding = 50.0f;
#endif
    range.halfDim = range.halfDim + arbitraryPadding;
    Rect **arr = GetRectsWithInRange(gameMetadata->sm, &range);
    AddDebugRect(gameMetadata, &range, COLOR_GREEN_TRANSPARENT);

    AABB uiTest = {};
    glm::vec3 rayWorld =
        UnProject(g_camera, g_projection, V2(g_mousePoint), gameMetadata->screenResolution);

    f32 dimRange = 0.1f;
    uiTest.halfDim = v2{dimRange, dimRange * (gameMetadata->screenResolution.x / gameMetadata->screenResolution.y)};
    uiTest.center = V2(ScreenSpaceToNormalizedDeviceSpace(V2(g_mousePoint), gameMetadata->screenResolution));
    AddDebugRectUI(gameMetadata, &uiTest, COLOR_RED_TRANSPARENT);

    /* This points towards us */
    glm::vec3 infinitePlaneNormal = glm::vec3(0, 0, 1);

    glm::vec3 worldPos =
        GetWorldPointFromMouse(
                g_camera,
                g_projection,
                V2(g_mousePoint),
                gameMetadata->screenResolution,
                infinitePlaneNormal);

    uiTest.halfDim = v2{dimRange, dimRange};
    uiTest.center = v2{worldPos.x, worldPos.y};
#if 0
    printf("coords: X %f, Y %f\n", uiTest.center.x, uiTest.center.y);
#endif

    AddDebugRect(gameMetadata, &uiTest, COLOR_BLUE_TRANSPARENT);

    for(memory_index i = 0; i < ARRAY_LIST_SIZE(arr); i++)
    {
        Rect *rect = arr[i];
        AddDebugRect(gameMetadata, rect, COLOR_YELLOW_TRANSPARENT);

        glm::vec3 rayDirection = glm::vec3(1 / rayWorld.x, 1 / rayWorld.y, 0);
        if (IntersectionAABB(rect, V2(g_camera->pos), rayDirection))
        {
            AddDebugRect(gameMetadata, rect, COLOR_BLACK_TRANSPARENT);
        }

#if 0
        real32 dX = e->velocity.x * dt;
        real32 dY = e->velocity.y + (gravity + e->acceleration.y) * dt;

        glm::vec3 rayDirection = { dX, dY, 0 };
        rayDirection = glm::normalize(rayDirection);
        rayDirection = glm::vec3(1 / rayDirection.x, 1 / rayDirection.y, 0);
#endif
        if (rect->type == COLLISION && TestAABBAABB(rect, &nextUpdate))
        {
            /* how to differentiate between x and y axis? */
            e->position.y = rect->max.y + gameMetadata->playerRect->height * 0.5f;
            e->velocity.y = 0;
            e->acceleration.y = gravity;
        }
    }

    /* TODO: There is a bug here. We're not properly updating the position
     * based on the collisions
     */
    e->position += e->velocity * dt + 0.5f * e->acceleration * dt * dt;
    e->velocity.y += e->acceleration.y *dt;

    UpdatePositionBasedOnCollission(g_sceneManager, g_enemyNPC, gravity, dt);

    /* TODO: can't let the player spam attack while we're still in an attack animation */
    if (e->willAttack)
    {
        /* This is related to hit boxes of the frames */
        if (e->frameState.startFrame == NULL)
        {
            e->frameState.startFrame =
                (FrameData *)AllocateMemory(reservedMemory, sizeof(FrameData));

            e->frameState.startFrame->duration = 170;
            e->frameState.startFrame->dim = v2{0.35f, 0.175f};
            e->frameState.startFrame->pos = v3{1.5, 0, 0};
            e->frameState.startFrame->next = NULL;

            e->frameState.currentFrame = e->frameState.startFrame;

            /* TODO: This should be somewhere else */
            e->frameState.transform = glm::mat4(-2.0f);
            e->frameState.transform[3][3] = 1.0f;

            if (gameMetadata->playerRect->frameDirection == RIGHT)
            {
                e->frameState.transform[0][0] = abs(e->frameState.transform[0][0]);
                e->frameState.transform[1][1] = abs(e->frameState.transform[1][1]);
            }

        }

        if (e->frameState.timePassedCurrentFrame == 0)
        {
            g_spriteAnimation =
                GetSpriteAnimationInfo(
                        GetFrameAnimation(&gameMetadata->frameAnimationSentinelNode, "arche.png"),
                        "ATTACK");
        }

        //AttackInfo attackInfo = GetAttackFrameInfo();
        if (UpdateFrameState(&e->frameState, gt->deltaTime))
        {
            /* TODO: The position that's passed needs to be the center of the
             * entity. Otherwise, it's hard to make the frame distance
             * symetrical when swapping between both directions
             */
            AddDebugRect(gameMetadata, &e->frameState, V3(e->position), COLOR_RED_TRANSPARENT);
        }
        else
        {
            e->willAttack = false;
        }
    }

    if (g_spriteAnimation->currentFrameIndex == 0 && e->willAttack == false)
    {
        g_spriteAnimation =
            GetSpriteAnimationInfo(
                    GetFrameAnimation(&gameMetadata->frameAnimationSentinelNode, "arche.png"),
                    "IDLE");
    }

    for (int i = 0; i < hitBoxes->size; i++)
    {
        Rect *rect = hitBoxes->rects[i];
        for (int y = 0; y < hurtBoxes->size; y++)
        {
            Rect hb = *(hurtBoxes->rects[y]);
            if (TestAABBAABB(rect, &hb))
            {
                printf("CHECKING hitboxes\n");
            }
        }
    }

    if (isPlayer)
    {
        /* TODO: bound checking for the camera such that we only move the camera
         * when necessary
         */
        //UpdateEntityFrameDirection();
        if (e->velocity.x > 0)
        {
            gameMetadata->playerRect->frameDirection = RIGHT;
            gameMetadata->playerRect->entity->frameState.transform[0][0] =
                 abs(gameMetadata->playerRect->entity->frameState.transform[0][0]);
            gameMetadata->playerRect->entity->frameState.transform[1][1] =
                 abs(gameMetadata->playerRect->entity->frameState.transform[1][1]);
        }
        else if (e->velocity.x < 0)
        {
            gameMetadata->playerRect->frameDirection = LEFT;
            gameMetadata->playerRect->entity->frameState.transform[0][0] =
                -1 * abs(gameMetadata->playerRect->entity->frameState.transform[0][0]);
            gameMetadata->playerRect->entity->frameState.transform[1][1] =
                -1 * abs(gameMetadata->playerRect->entity->frameState.transform[1][1]);
        }
        /* else don't update */

        /* follow the character around */
        CameraUpdateTarget(g_camera, e->position);
        UpdatePosition(gameMetadata->playerRect, v3{ e->position.x, e->position.y, e->position.z });

        UpdateCurrentFrame(g_spriteAnimation, 17.6f);
        UpdateUV(g_rectManager->player, *g_spriteAnimation->currentFrame);
        UpdateFrameDirection(g_spriteAnimation, gameMetadata->playerRect->frameDirection);
    }

    /* Apply "friction" */
    e->velocity.x = 0;

    for (memory_index i = 0; i < gameMetadata->particleSystem.particleCount; i++)
    {
        Particle *particle = &gameMetadata->particleSystem.particles[i]; // NOLINT

        v3 newPosition = v3{ e->position.x,
                             e->position.y,
                             e->position.z};

        CreateVertices(&particle->rect);
        particle->acc.y = gravity * 2;
        particle->vel.x = 0;
        UpdateParticlePosition(particle, dt);
        if (particle->positionOffset.y <= gameMetadata->playerRect->min.y)
        {
            particle->positionOffset.y = newPosition.y;
            particle->acc.y = 0;
            UpdatePosition(&particle->rect, particle->positionOffset);
        }
        else
        {
            UpdatePosition(&particle->rect, particle->positionOffset + newPosition);
        }

    }
    UpdateAndGenerateParticleRectInfo(perFrameRenderGroup,
                                      &gameMetadata->particleSystem,
                                      (s32)gameMetadata->gameTimestep->deltaTime);
}

void Update(GameMetadata *gameMetadata, GameTimestep *gameTimestep, RectDynamicArray *hitBoxes, RectDynamicArray *hurtBoxes, RenderGroup *perFrameRenderGroup)
{

    /* update logics and data here */
    /* physics */
    UpdateGameTimestep(gameTimestep);

    const GLfloat gravity = -9.81f;
    UpdateNPCMovement(g_enemyNPC, gameMetadata, gameTimestep->dt);

    /* Update entities */
    UpdateEntities(gameMetadata, gameTimestep, hitBoxes, hurtBoxes, perFrameRenderGroup, true);
}

void DrawScene(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        VulkanContext *vc,
        VulkanBuffers *vkBuffers,
        Camera *camera,
        glm::mat4 *projection,
        RectDynamicArray *hitBoxes,
        RectDynamicArray *hurtBoxes)
{
    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;

    ASSERT(g_rectManager->Traversable.rda.size > 0);

    AABB range = {};
    range.halfDim = V2(gameMetadata->mouseDrag[0] - gameMetadata->mouseDrag[1]) * 0.5f;
    range.center = V2(gameMetadata->mouseDrag[0]) - range.halfDim;
    /* sometimes we're going to the negative value when calculating the
     * dimension. This needs to be positive in order to work.
     * We also need to use the negative dimension to calculate two out of the four quadrants.
     */
    range.halfDim = abs(range.halfDim);
    Rect *dragCreatedRect = CreateMinimalRectInfo(&gameMetadata->reservedMemory, COLOR_BLUE_TRANSPARENT, &range);

    RectDynamicArray *newWorldObjects = CreateRDAForNewWorldObjects(gameMetadata);

    if (gameMetadata->createNewRect)
    {
        gameMetadata->createNewRect = false;

        Rect *permanentRect = CreateMinimalRectInfo(&gameMetadata->reservedMemory, COLOR_BLUE_TRANSPARENT, &range);
        permanentRect->type = COLLISION;
        permanentRect->bitmapID = FindBitmap(&gameMetadata->bitmapSentinelNode, "box")->bitmapID;
        permanentRect->renderLayer = FRONT_STATIC;
        PushBack(&g_rectManager->NonTraversable.rda, permanentRect);

        SetAABB(&g_rectManager->NonTraversable);
    }

    AddDebugRect(gameMetadata, &range, COLOR_RED_TRANSPARENT);

    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, &g_rectManager->Traversable.rda);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, &g_rectManager->NonTraversable.rda);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, newWorldObjects);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, hitBoxes);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, hurtBoxes);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, gameMetadata->rdaDebug);

    PushRenderGroupRectInfo(perFrameRenderGroup, gameMetadata->playerRect);

    Rect *minimalRect = CreateMinimalRectInfo(perFrameMemory, g_enemyNPC);
    UpdateNPCAnimation(g_enemyNPC, minimalRect);
    PushRenderGroupRectInfo(perFrameRenderGroup, minimalRect);

    ClearUsedVertexRenderGroup(perFrameRenderGroup);

#if 1
    Rect **sortedRectInfo = MergeSortRenderGroupRectInfo(perFrameRenderGroup, perFrameMemory);
#else
    QuickSortRenderGroupRectInfo(perFrameRenderGroup);
#endif

    UpdateUBOandPushConstants(
            gameMetadata,
            vc,
            camera,
            projection);

    DrawRenderGroup(
            gameMetadata,
            perFrameRenderGroup,
            sortedRectInfo,
            vc,
            vkBuffers);

    ClearUsedVertexRenderGroup(perFrameRenderGroup);
    ClearUsedRectInfoRenderGroup(perFrameRenderGroup);
}

void DrawUI(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        VulkanContext *vc,
        VulkanBuffers *vkBuffers,
        const f64 &MSPerFrame,
        const f64 &FPS,
        const f64 &MCPF)
{

    char buffer[256];
    Bitmap stringBitmap = {};
    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;

    f32 screenWidth = gameMetadata->screenResolution.v[0];
    f32 screenHeight = gameMetadata->screenResolution.v[1];

    sprintf_s(buffer, sizeof(char) * 150, "  %.02f ms/f    %.0ff/s    %.02fcycles/f  ", MSPerFrame, FPS, MCPF); // NOLINT

    f32 scaleFactor = 0.25;

    /* This is in raw OpenGL coordinates */
    v3 startingPosition = v3{ -1, 1 - gameMetadata->fontBitmap.height / screenHeight * scaleFactor * 0.5f, 0 };

    PushStringRectToRenderGroup(
            perFrameRenderGroup, gameMetadata, perFrameMemory, startingPosition, scaleFactor, buffer);

    if (gameMetadata->isEditMode)
    {
        f32 rectHeight = gameMetadata->fontBitmap.height / screenHeight * scaleFactor * 0.5f;
        f32 padding = 0.0f;
        startingPosition =
            v3{ 0.0f, -1 + rectHeight, 0 };
        Rect *bottomUIBar =
            CreateRectangle(perFrameMemory, startingPosition, COLOR_WHITE, 2, rectHeight * 2);
        bottomUIBar->bitmapID = gameMetadata->whiteBitmap.bitmapID;
        bottomUIBar->bitmap = &gameMetadata->whiteBitmap;
        PushRenderGroupRectInfo(perFrameRenderGroup, bottomUIBar);

        padding = 0.02f;
        startingPosition = v3{ -1 + padding, -1 + rectHeight, 0 };
        PushStringRectToRenderGroup(
                perFrameRenderGroup, gameMetadata, perFrameMemory, startingPosition, scaleFactor, "EDITMODE");
    }

    perFrameRenderGroup->rectCount = 0;
    ClearMemoryUsed(&perFrameRenderGroup->vertexMemory);

    /* TODO: Update UI texture */
    /* use texture of arrays or arrays of texture? */
    vkCmdNextSubpass(vc->drawCmd, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(vc->drawCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vc->pipeline2);
    vkCmdBindDescriptorSets(
            vc->drawCmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vc->pipelineLayout,
            0,
            1,
            &vc->secondDescSet,
            0,
            nullptr);
#if 0
    VulkanCopyImageFromHostToLocal(
            &vc-> device,
            stringBitmap.pitch,
            stringBitmap.height,
            vc->UITextures[0].image,
            vc->UITextures[0].mem,
            stringBitmap.size,
            stringBitmap.data);
#endif

    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, gameMetadata->rdaDebugUI);

    DrawRenderGroup(
            gameMetadata,
            perFrameRenderGroup,
            vc,
            vkBuffers);
}

void Render(GameMetadata *gameMetadata,
        RectDynamicArray *hitBoxes,
        RectDynamicArray *hurtBoxes,
        RenderGroup *perFrameRenderGroup,
        VulkanContext *vc)
{

    f64 MSPerFrame = 0;
    f64 FPS = 0;
    f64 MCPF = 0;
    CalculateFrameStatistics(gameMetadata->gameTimestep, &MSPerFrame, &FPS, &MCPF);

    VulkanPrepareRender(vc);
    VulkanBeginRenderPass(vc);
    VulkanSetViewportAndScissor(vc);

    /* Should I differentiate between scene and ui rendergroups? */
    DrawScene(
        gameMetadata,
        perFrameRenderGroup,
        vc,
        &g_vkBuffers,
        g_camera,
        g_projection,
        hitBoxes,
        hurtBoxes);

    DrawUI(
        gameMetadata,
        perFrameRenderGroup,
        vc,
        &g_vkBuffers,
        MSPerFrame,
        FPS,
        MCPF);

    EndRender(gameMetadata, vc);
    RenderCleanup(gameMetadata, vc, &g_vkBuffers);
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

            Entity *collisionEntity =
                AddNewEntity(reservedMemory, g_entityManager, startingPosition);
            ASSERT(collisionEntity != nullptr);
            collisionEntity->isTraversable = false;
            collisionEntity->isPlayer = false;

            Rect *collissionRect =
                CreateRectangle(reservedMemory, startingPosition, color, currentNode->dim);
            AssociateEntity(collissionRect, collisionEntity, false);

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
