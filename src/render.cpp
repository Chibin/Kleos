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
#include <GL/gl.h>
#include <GL/glu.h>

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
#include "opengl.cpp"
#include "shaders.cpp"

#include "collision.cpp"
#include "rect_manager.cpp"

#include "rectangle.cpp"

#include "particle.cpp"
#include "render_group.h"
#include "asset.cpp"
#include "sort.cpp"

#include "hashtable.cpp"

#include "renderer/vulkan/my_vulkan.cpp"
#include "renderer/common.cpp"

#define COLOR_WHITE \
    v4              \
    {               \
        1, 1, 1, 1  \
    }
#define COLOR_RED  \
    v4             \
    {              \
        1, 0, 0, 1 \
    }
#define COLOR_GREEN \
    v4              \
    {               \
        0, 1, 0, 1  \
    }
#define COLOR_BLUE \
    v4             \
    {              \
        0, 0, 1, 1 \
    }
#define COLOR_BLACK \
    v4              \
    {               \
        0, 0, 0, 1  \
    }

#include "npc.cpp"

#define UPDATEANDRENDER(name) \
    bool name(GameMetadata *gameMetadata)
#define RENDER(name)                                                    \
    void name(GLuint vao, GLuint vbo, GLuint textureID, GLuint program, \
              GLuint debugProgram, Entity *entity)

void Render(GameMetadata *gameMetadata, GLuint vao, GLuint vbo, GLuint textureID, GLuint program,
            GLuint debugProgram, Entity *entity, RectDynamicArray *hitBoxes, RectDynamicArray *hurtBoxes,
            RenderGroup *perFrameRenderGroup, VulkanContext *vc);
void Update(GameMetadata *gameMetadata, GameTimestep *gameTimestep, RectDynamicArray *hitBoxes,
            RectDynamicArray *hurtBoxes, RenderGroup *perFrameRenderGroup);
void LoadStuff(GameMetadata *gameMetadata);
inline void LoadShaders(GameMetadata *gameMetadata);
inline void LoadAssets(GameMetadata *gameMetadata);
inline void SetOpenGLDrawToScreenCoordinate(GLuint projectionLoc, GLuint viewLoc);

/* TODO: We'll need to get rid of these global variables later on */
Camera *g_camera = nullptr;
glm::mat4 *g_projection = nullptr;
GLfloat *g_rectangleVertices = nullptr;
EntityManager *g_entityManager = nullptr;
Entity *g_player = nullptr;
RectManager *g_rectManager = nullptr;
EntityDynamicArray *g_eda = nullptr;
v3 g_mousePoint;
static bool g_debugMode = false;
static bool g_spriteDirectionToggle = false;
static Animation2D *g_spriteAnimation = nullptr;
static Animation2D *g_spriteAnimations = nullptr;
static GLuint g_permanentTextureID;
static VulkanBuffers g_vkBuffers;
static memory_index g_bitmapID = 0;
static NPC *g_enemyNPC = nullptr;

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

    if (gameMetadata->isVulkanActive)
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

        gameMetadata->whiteBitmap.width = 1;
        gameMetadata->whiteBitmap.height = 1;
        gameMetadata->whiteBitmap.format = GL_RGBA;
        gameMetadata->whiteBitmap.bitmapID = 998; // XXX: needs to be changed somewhere
        gameMetadata->whiteBitmap.textureParam = TextureParam{ GL_NEAREST, GL_NEAREST };
        gameMetadata->whiteBitmap.data =
            (u8 *)AllocateMemory(reservedMemory, gameMetadata->whiteBitmap.width * gameMetadata->whiteBitmap.height * sizeof(u32));
        for (memory_index i = 0; i < gameMetadata->whiteBitmap.width * gameMetadata->whiteBitmap.height; i++)
        {
            /* alpha -> blue -> green -> red: 1 byte each */
            *((u32 *)gameMetadata->whiteBitmap.data + i) = 0x33333333;
        }

        TTF_Font *font = OpenFont();
        ASSERT(font != nullptr);
        gameMetadata->font = font;
        //ASSERT(TTF_FontFaceIsFixedWidth(font) == 1);

        g_vkBuffers.count = 0;
        g_vkBuffers.maxNum = 100;

        Bitmap stringBitmap = {};
        if (gameMetadata->isVulkanActive)
        {

            memset(&gameMetadata->bitmapToDescriptorSetMap, 0, sizeof(BitmapDescriptorMap));
#if 1
            for (memory_index i = 0; i < MAX_HASH; i++)
            {
                ASSERT(gameMetadata->bitmapToDescriptorSetMap.hashTable[i] == nullptr);
            }
#endif
            // BitmapDescriptorMap bitmapToDescriptorSetMap  = CreateNewHashMap();
            bool useStagingBuffer = false;
            stbi_set_flip_vertically_on_load(1);

            s32 texWidth = 0;
            s32 texHeight = 0;
            s32 texChannels = 0;
            stbi_uc *pixels = stbi_load("./materials/textures/awesomeface.png",
                    &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            vc->textures[0].texWidth = texWidth;
            vc->textures[0].texHeight = texHeight;
            /* stbi does not do padding, so the pitch is the component times
             * the width of the image. The component is 4 because of STBI_rgb_alpha.
             */
            vc->textures[0].texPitch = texWidth * 4;
            vc->textures[0].dataSize = texWidth * texHeight * 4;
            vc->textures[0].data = pixels;

            VulkanPrepareTexture(vc,
                    &vc->gpu,
                    &vc->device,
                    &vc->setupCmd,
                    &vc->cmdPool,
                    &vc->queue,
                    &vc->memoryProperties,
                    useStagingBuffer,
                    vc->textures,
                    VK_IMAGE_LAYOUT_GENERAL);;

            stbi_image_free(pixels);

            //
            // bitmap
            // descriptor set
            //

            stringBitmap = {};
            stringBitmap.bitmapID = 99; // Arbitrary number
            char buffer[256];
            /* Hack: There's a bunch of blank spaces at the end to accomodate
             * the amout of extra characters for later images.
             * This will give us a longer width when creating a vkimage.
             */
            sprintf_s(buffer, sizeof(char) * 150, "   %.02f ms/f    %.0ff/s    %.02fcycles/f              ", 33.0f, 66.0f, 99.0f); // NOLINT
            StringToBitmap(perFrameMemory, &stringBitmap, gameMetadata->font, buffer);                                             // NOLINT
            stringBitmap.textureParam = TextureParam{ GL_NEAREST,  GL_NEAREST };

            vc->UITextures[0].texWidth = stringBitmap.width;
            vc->UITextures[0].texHeight = stringBitmap.height;
            vc->UITextures[0].texPitch = stringBitmap.pitch;
            vc->UITextures[0].dataSize = stringBitmap.size;
            vc->UITextures[0].data = stringBitmap.data;

            VulkanPrepareTexture(vc,
                    &vc->gpu,
                    &vc->device,
                    &vc->setupCmd,
                    &vc->cmdPool,
                    &vc->queue,
                    &vc->memoryProperties,
                    useStagingBuffer,
                    vc->UITextures,
                    VK_IMAGE_LAYOUT_GENERAL);

            stbi_uc *playerPixels = stbi_load( "./materials/textures/arche.png",
                    &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            vc->playerTextures[0].texWidth = texWidth;
            vc->playerTextures[0].texHeight = texHeight;
            /* stbi does not do padding, so the pitch is the component times
             * the width of the image. The component is 4 because of STBI_rgb_alpha.
             */
            vc->playerTextures[0].texPitch = texWidth * 4;
            vc->playerTextures[0].dataSize = texWidth * texHeight * 4;
            vc->playerTextures[0].data = playerPixels;

            VulkanPrepareTexture(vc,
                    &vc->gpu,
                    &vc->device,
                    &vc->setupCmd,
                    &vc->cmdPool,
                    &vc->queue,
                    &vc->memoryProperties,
                    useStagingBuffer,
                    vc->playerTextures,
                    VK_IMAGE_LAYOUT_GENERAL);

            stbi_image_free(playerPixels);

            stbi_uc *boxPixels = stbi_load( "./materials/textures/container.png",
                    &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            vc->boxTextures[0].texWidth = texWidth;
            vc->boxTextures[0].texHeight = texHeight;
            /* stbi does not do padding, so the pitch is the component times
             * the width of the image. The component is 4 because of STBI_rgb_alpha.
             */
            vc->boxTextures[0].texPitch = texWidth * 4;
            vc->boxTextures[0].dataSize = texWidth * texHeight * 4;
            vc->boxTextures[0].data = boxPixels;

            VulkanPrepareTexture(vc,
                    &vc->gpu,
                    &vc->device,
                    &vc->setupCmd,
                    &vc->cmdPool,
                    &vc->queue,
                    &vc->memoryProperties,
                    useStagingBuffer,
                    vc->boxTextures,
                    VK_IMAGE_LAYOUT_GENERAL);

            stbi_image_free(boxPixels);

            vc->whiteTextures[0].texWidth = gameMetadata->whiteBitmap.width;
            vc->whiteTextures[0].texHeight = gameMetadata->whiteBitmap.height;
            /* stbi does not do padding, so the pitch is the component times
             * the width of the image. The component is 4 because of STBI_rgb_alpha.
             */
            vc->whiteTextures[0].texPitch = gameMetadata->whiteBitmap.width * 4;
            vc->whiteTextures[0].dataSize = gameMetadata->whiteBitmap.width * gameMetadata->whiteBitmap.height * 4;
            vc->whiteTextures[0].data = gameMetadata->whiteBitmap.data;

            VulkanPrepareTexture(vc,
                    &vc->gpu,
                    &vc->device,
                    &vc->setupCmd,
                    &vc->cmdPool,
                    &vc->queue,
                    &vc->memoryProperties,
                    useStagingBuffer,
                    vc->whiteTextures,
                    VK_IMAGE_LAYOUT_GENERAL);

            /* Creating pipeline layout, descriptor pool, and render pass can be done
             * indenpendently
             */
            VulkanPrepareDescriptorPool(vc);

            vc->pipelineLayout = {};
            VulkanInitPipelineLayout(vc);
            VulkanSetDescriptorSet(
                    vc,
                    &vc->descSet,
                    vc->textures,
                    DEMO_TEXTURE_COUNT,
                    &vc->uniformData,
                    &vc->uniformDataFragment);

            VulkanSetDescriptorSet(
                    vc,
                    &vc->secondDescSet,
                    vc->UITextures,
                    DEMO_TEXTURE_COUNT,
                    &vc->uniformData,
                    &vc->uniformDataFragment);

            VulkanSetDescriptorSet(
                    vc,
                    &vc->playerDescSet,
                    vc->playerTextures,
                    DEMO_TEXTURE_COUNT,
                    &vc->uniformData,
                    &vc->uniformDataFragment);

            VulkanSetDescriptorSet(
                    vc,
                    &vc->boxDescSet,
                    vc->boxTextures,
                    DEMO_TEXTURE_COUNT,
                    &vc->uniformData,
                    &vc->uniformDataFragment);

            VulkanSetDescriptorSet(
                    vc,
                    &vc->whiteDescSet,
                    vc->whiteTextures,
                    DEMO_TEXTURE_COUNT,
                    &vc->uniformData,
                    &vc->uniformDataFragment);

            VulkanInitRenderPass(vc);
            VulkanInitFrameBuffers(vc);

            memset(&vc->vertices, 0, sizeof(vc->vertices));
            VulkanPreparePipeline(vc, sizeof(Vertex));
            VulkanPrepare2ndPipeline(vc, sizeof(Vertex));
        }

        START_DEBUG_TIMING();

        ASSERT(!*gameTimestep);
        ASSERT(!g_camera);
        ASSERT(!g_eda);
        ASSERT(!g_projection);
        ASSERT(!g_rectManager);
        ASSERT(!g_entityManager);
        ASSERT(!g_reservedMemory);

        glGenTextures(1, &g_permanentTextureID);

        g_reservedMemory = reservedMemory;

        *gameTimestep = (GameTimestep *)AllocateMemory(reservedMemory, sizeof(GameTimestep));
        ResetGameTimestep(*gameTimestep);

        v3 cameraPos = { 0, 0, 10 };
        // and looks at the origin
        v3 cameraTarget = { 0, 0, 0 };
        // Head is up (set to 0,-1,0 to look upside-down)
        v3 cameraUp = { 0, 1, 0 };
        g_camera = CreateCamera(reservedMemory, cameraPos, cameraTarget, cameraUp);

        g_eda = CreateEntityDynamicArray(reservedMemory);

        f32 screenWidth = screenResolution.v[0];
        f32 screenHeight = screenResolution.v[1];
        g_projection = (glm::mat4 *)AllocateMemory(reservedMemory, (sizeof(glm::mat4)));
        *g_projection =
            glm::infinitePerspective(45.0f, screenWidth / screenHeight, 0.1f);

        g_rectManager = CreateRectManager(reservedMemory);

        /* TODO: May be the entity manager should be the only one creating the
         * entities?
         */
        g_entityManager = CreateEntityManger(reservedMemory);

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
        gameMetadata->playerRect->frameDirection = LEFT;

        LoadAssets(gameMetadata);

        if (gameMetadata->isVulkanActive)
        {
            HashInsert(
                    &gameMetadata->bitmapToDescriptorSetMap,
                    FindBitmap(&gameMetadata->bitmapSentinelNode, "box"),
                    &vc->boxDescSet);

            HashInsert(
                    &gameMetadata->bitmapToDescriptorSetMap,
                    FindBitmap(&gameMetadata->bitmapSentinelNode, "arche"),
                    &vc->playerDescSet);

            HashInsert(
                    &gameMetadata->bitmapToDescriptorSetMap,
                    FindBitmap(&gameMetadata->bitmapSentinelNode, "awesomeface"),
                    &vc->descSet);

            HashInsert(
                    &gameMetadata->bitmapToDescriptorSetMap,
                    &gameMetadata->whiteBitmap,
                    &vc->whiteDescSet);

            HashInsert(
                    &gameMetadata->bitmapToDescriptorSetMap,
                    &stringBitmap,
                    &vc->secondDescSet);
        }


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
        gameMetadata->particleSystem.particleCount = 1000;
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
        case SDL_MOUSEWHEEL:
            ProcessMouseInput(event, g_camera);
            break;
        case SDL_MOUSEMOTION:
            g_mousePoint = ProcessMouseMotion(event.motion, g_camera);
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

    if (gameMetadata->isOpenGLActive)
    {

        if (!gameMetadata->initOpenGL)
        {
            gameMetadata->initOpenGL = true;

            LoadShaders(gameMetadata);

            /*  Each vertex attribute takes its data from memory managed by a
             *  VBO. VBO data -- one could have multiple VBOs -- is determined by the
             *  current VBO bound to GL_ARRAY_BUFFER when calling glVertexAttribPointer.
             *  Since the previously defined VBO was bound before
             *  calling glVertexAttribPointer vertex attribute 0 is now associated with
             * its vertex data.
             */

            glGenVertexArrays(1, &gameMetadata->vaoID);
            glGenBuffers(1, &gameMetadata->eboID);
            glGenBuffers(1, &gameMetadata->vboID);

            OpenGLCreateVAO(
                    gameMetadata->vaoID,
                    gameMetadata->vboID,
                    sizeof(Vertex) * NUM_OF_RECT_CORNER,
                    nullptr,   /* use null to skip preloading data to vbo */
                    gameMetadata->eboID, sizeof(g_rectIndices),
                    g_rectIndices); // NOLINT
        }

        /* start with a 'clear' screen */
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        //glEnable(GL_DEPTH_TEST);
        /* Ignore depth values (Z) to cause drawing bottom to top */
        //glDepthFunc(GL_NEVER);
        /* same as the one above? */
        glDisable (GL_DEPTH_TEST);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // NOLINT
    }

    if (gameMetadata->isOpenGLActive)
    {
        ASSERT(gameMetadata->program);
        ASSERT(gameMetadata->debugProgram);
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

    Update(gameMetadata, *gameTimestep, hitBoxes, hurtBoxes, &perFrameRenderGroup);
    Render(gameMetadata,
           gameMetadata->vaoID,
           gameMetadata->vboID,
           gameMetadata->textureID,
           gameMetadata->program,
           gameMetadata->debugProgram,
           g_player,
           hitBoxes,
           hurtBoxes,
           &perFrameRenderGroup,
           vc);

    return continueRunning;
}

void UpdateEntities(GameMetadata *gameMetadata, GameTimestep *gt, RectDynamicArray *hitBoxes, RectDynamicArray *hurtBoxes, RenderGroup *perFrameRenderGroup, bool isPlayer = false)
{
    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;
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
    v2 center = {0.5f * gameMetadata->playerRect->width, 0.5f * gameMetadata->playerRect->height};
    Rect nextUpdate = *gameMetadata->playerRect;
    nextUpdate.min = v2{e->position.x + e->velocity.x * dt,
        e->position.y + e->velocity.y * dt + 0.5f * e->acceleration.y * dt * dt} - center;
    nextUpdate.max = v2{e->position.x + e->velocity.x * dt,
        e->position.y + e->velocity.y * dt + 0.5f * e->acceleration.y * dt * dt} + center;

    for (int i = 0; i < g_rectManager->NonTraversable.size; i++)
    {
        /* This will need to happen for all AABB checks */
        Rect *rect = g_rectManager->NonTraversable.rects[i];

        real32 dX = e->velocity.x * dt;
        real32 dY = e->velocity.y + (gravity + e->acceleration.y) * dt;

        glm::vec3 rayDirection = { dX, dY, 0 };
        rayDirection = glm::normalize(rayDirection);
        rayDirection = glm::vec3(1 / rayDirection.x, 1 / rayDirection.y, 0);

        /* This would be nice to be put into the screen instead of the console
         */
        printf("rect X- min: %f max: %f\n", rect->min.x, rect->max.x);
        printf("rect Y- min: %f max: %f\n", rect->min.y, rect->max.y);

        if (rect->type == COLLISION && TestAABBAABB(rect, &nextUpdate))
        {
            /* how to differentiate between x and y axis? */
            printf("I hit something!!!!!!!\n");
            e->position.y = rect->max.y + gameMetadata->playerRect->height * 0.5f;
            e->velocity.y = 0;
            e->acceleration.y = 0;
            e->position.x += e->velocity.x * dt;
        }
        else
        {
            if (rect->type == HITBOX)
            {
                PushBack(hitBoxes, rect);
            }
            else if (rect->type == HURTBOX)
            {
                PushBack(hurtBoxes, rect);
            }
        }
    }

    /* TODO: There is a bug here. We're not properly updating the position
     * based on the collisions
     */
    e->acceleration.y += gravity;
    e->velocity.y += e->acceleration.y * dt;
    e->position.y += e->velocity.y * dt;
    e->position.x += e->velocity.x * dt;

    UpdatePositionBasedOnCollission(g_enemyNPC, g_rectManager, gravity, dt);

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
            Rect *attackHitBox = CreateFrameRect(perFrameMemory,
                    &e->frameState,
                    v3{ e->position.x, e->position.y, e->position.z},
                    COLOR_RED);
            PushBack(hitBoxes, attackHitBox);
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
    UpdateNPCMovement(g_enemyNPC, g_rectManager, gravity, gameTimestep->dt);

    /* Update entities */
    UpdateEntities(gameMetadata, gameTimestep, hitBoxes, hurtBoxes, perFrameRenderGroup, true);
}

void DrawScene(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        VulkanContext *vc,
        VulkanBuffers *g_vkBuffers,
        Camera *g_camera,
        glm::mat4 *g_projection,
        GLuint *viewLoc,
        GLuint *projectionLoc,
        GLuint *textureID,
        RectDynamicArray *hitBoxes,
        RectDynamicArray *hurtBoxes)
{
    ASSERT(g_rectManager->Traversable.size > 0);
    for (memory_index i = 0; i < g_rectManager->Traversable.size; i++)
    {
        Rect *rect = g_rectManager->Traversable.rects[i];
        rect->bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, rect->bitmapID);
        ASSERT(rect->bitmap);
        PushRenderGroupRectInfo(perFrameRenderGroup, rect);
    }

    for (int i = 0; i < g_rectManager->NonTraversable.size; i++)
    {
        Rect *rect = g_rectManager->NonTraversable.rects[i];
        rect->bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, rect->bitmapID);
        ASSERT(rect->bitmap);
        PushRenderGroupRectInfo(perFrameRenderGroup, rect);
    }

    for (memory_index i = 0; i < hitBoxes->size; i++)
    {
        Rect *rect = hitBoxes->rects[i];
        rect->bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, rect->bitmapID);
        ASSERT(rect->bitmap);
        PushRenderGroupRectInfo(perFrameRenderGroup, rect);
    }

    for (memory_index i = 0; i < hurtBoxes->size; i++)
    {
        Rect *rect = hurtBoxes->rects[i];
        rect->bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, rect->bitmapID);
        ASSERT(rect->bitmap);
        PushRenderGroupRectInfo(perFrameRenderGroup, rect);
    }

    gameMetadata->playerRect->bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode,
                                                  gameMetadata->playerRect->bitmapID);

    PushRenderGroupRectInfo(perFrameRenderGroup, gameMetadata->playerRect);

    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;
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
            g_camera,
            g_projection,
            viewLoc,
            projectionLoc);

    DrawRenderGroup(
            gameMetadata,
            perFrameRenderGroup,
            sortedRectInfo,
            vc,
            g_vkBuffers,
            g_camera,
            g_projection,
            textureID);

    ClearUsedVertexRenderGroup(perFrameRenderGroup);
    ClearUsedRectInfoRenderGroup(perFrameRenderGroup);
}

void DrawUI(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        VulkanContext *vc,
        VulkanBuffers *g_vkBuffers,
        Camera *g_camera,
        glm::mat4 *g_projection,
        GLuint *viewLoc,
        GLuint *projectionLoc,
        GLuint *textureID,
        const f64 &MSPerFrame,
        const f64 &FPS,
        const f64 &MCPF)
{

    char buffer[256];
    Bitmap stringBitmap = {};
    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;

    /* TODO: create something that can get a new bitmap id? */
    stringBitmap.bitmapID = 99;
    sprintf_s(buffer, sizeof(char) * 150, "  %.02f ms/f    %.0ff/s    %.02fcycles/f  ", MSPerFrame, FPS, MCPF); // NOLINT
    StringToBitmap(perFrameMemory, &stringBitmap, gameMetadata->font, buffer);                                  // NOLINT

    ASSERT(vc->UITextures[0].texWidth >= stringBitmap.width);
    ASSERT(vc->UITextures[0].texHeight >= stringBitmap.height);

    stringBitmap.textureParam = TextureParam{ GL_NEAREST,  GL_NEAREST };

    f32 screenWidth = gameMetadata->screenResolution.v[0];
    f32 screenHeight = gameMetadata->screenResolution.v[1];
    f32 scaleFactor = 3.0f;

    f32 rectWidth  = stringBitmap.width / screenWidth * scaleFactor;
    f32 rectHeight = stringBitmap.height / screenHeight * scaleFactor;

    /* This is in raw OpenGL coordinates */
    v3 startingPosition = v3{ -1 + rectWidth * 0.5f, 1 - rectHeight * 0.5f, 0 };

    Rect *statsRect =
        CreateRectangle(perFrameMemory, startingPosition, COLOR_WHITE, rectWidth, rectHeight);

    statsRect->bitmapID = gameMetadata->whiteBitmap.bitmapID;
    statsRect->bitmap = &gameMetadata->whiteBitmap;
    PushRenderGroupRectInfo(perFrameRenderGroup, statsRect);

    statsRect->bitmap = &stringBitmap;

    perFrameRenderGroup->rectCount = 0;
    ClearMemoryUsed(&perFrameRenderGroup->vertexMemory);

    if (gameMetadata->isVulkanActive)
    {

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

        ASSERT(vc->UITextures[0].texWidth >= stringBitmap.width);
        ASSERT(vc->UITextures[0].texHeight >= stringBitmap.height);
        VulkanCopyImageFromHostToLocal(
                &vc-> device,
                stringBitmap.pitch,
                stringBitmap.height,
                vc->UITextures[0].image,
                vc->UITextures[0].mem,
                stringBitmap.size,
                stringBitmap.data);

        /* I am doing some "shady" things in the vulkan side.
         * There's a fixed sized image that we're always updating.
         * sometimes, the new texture is smaller than the fixed size image we have.
         * so we have to scale the UV coordinates based on the newly generated coordinates.
         * bleh... but it works -- kind of.
         * Ideally, the text overlay should be generated based on a generated glyph.
         */
        Vertex *vTopRight = &(statsRect->vertices[0]);
        vTopRight->vUv = PixelToUV(v2{ (f32)stringBitmap.width, (f32)stringBitmap.height}, (u32)vc->UITextures[0].texWidth, (u32)vc->UITextures[0].texHeight);
        Vertex *vBottomRight = &(statsRect->vertices[1]);
        vBottomRight->vUv = PixelToUV(v2{ (f32)stringBitmap.width, 0}, (u32)vc->UITextures[0].texWidth, (u32)vc->UITextures[0].texHeight);
        Vertex *vBottomLeft = &(statsRect->vertices[2]);
        vBottomLeft->vUv = v2{ 0, 0 };
        Vertex *topLeft = &(statsRect->vertices[3]);
        topLeft->vUv = PixelToUV(v2{ 0, (f32)stringBitmap.height}, (u32)vc->UITextures[0].texWidth, (u32)vc->UITextures[0].texHeight);
    }

    /* UI has the highest priority on the screen, so it should be drawn last */
    if (gameMetadata->isOpenGLActive)
    {
        SetOpenGLDrawToScreenCoordinate(*viewLoc, *projectionLoc);
    }

    PushRenderGroupRectInfo(perFrameRenderGroup, statsRect);

    DrawRenderGroup(
            gameMetadata,
            perFrameRenderGroup,
            vc,
            g_vkBuffers,
            g_camera,
            g_projection,
            textureID);

}

void Render(GameMetadata *gameMetadata, GLuint vao, GLuint vbo, GLuint textureID, GLuint program,
            GLuint debugProgram, Entity *entity, RectDynamicArray *hitBoxes, RectDynamicArray *hurtBoxes,
            RenderGroup *perFrameRenderGroup, VulkanContext *vc)
{

    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;

    f64 MSPerFrame = 0;
    f64 FPS = 0;
    f64 MCPF = 0;
    CalculateFrameStatistics(gameMetadata->gameTimestep, &MSPerFrame, &FPS, &MCPF);

    GLuint modelLoc, viewLoc, projectionLoc;

    if (gameMetadata->isVulkanActive)
    {
        VulkanPrepareRender(vc);
        VulkanBeginRenderPass(vc);
        VulkanSetViewportAndScissor(vc);
    }

    if (gameMetadata->isOpenGLActive)
    {
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /* programs used first will have higher priority being shown in the
         * canvas
         */
        glBindVertexArray(vao);

        OpenGLCheckErrors();
        OpenGLBeginUseProgram(program, textureID);

        /* load uniform variable to shader program before drawing */
        modelLoc = glGetUniformLocation(program, "model");
        viewLoc = glGetUniformLocation(program, "view");
        projectionLoc = glGetUniformLocation(program, "projection");

        OpenGLCheckErrors();
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    /* Should I differentiate between scene and ui rendergroups? */
    DrawScene(
        gameMetadata,
        perFrameRenderGroup,
        vc,
        &g_vkBuffers,
        g_camera,
        g_projection,
        &viewLoc,
        &projectionLoc,
        &textureID,
        hitBoxes,
        hurtBoxes);

    DrawUI(
        gameMetadata,
        perFrameRenderGroup,
        vc,
        &g_vkBuffers,
        g_camera,
        g_projection,
        &viewLoc,
        &projectionLoc,
        &textureID,
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
            PushBack(&(g_rectManager->Traversable), r);
        }
    }

    /* Spawn new enemty */
    g_enemyNPC = (NPC *)AllocateMemory(reservedMemory, sizeof(NPC));
    memset(g_enemyNPC, 0, sizeof(NPC));

    g_enemyNPC->dim.height = 1.0f;
    g_enemyNPC->dim.width = 1.5f;
    g_enemyNPC->bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, "arche");
    g_enemyNPC->spriteAnimation =
        CreateCopyOfSpriteAnimationInfo(
                reservedMemory,
                GetSpriteAnimationInfo(
                    GetFrameAnimation(&gameMetadata->frameAnimationSentinelNode, "arche.png"),
                    "IDLE")
        );
    g_enemyNPC->spriteAnimation->direction = LEFT;
    g_enemyNPC->direction = LEFT;
    g_enemyNPC->renderLayer = BEHIND_PLAYER;
    g_enemyNPC->movementType = X_MOVEMENT;
    g_enemyNPC->movementPattern = UNI_DIRECTIONAL;
}

inline void SetOpenGLDrawToScreenCoordinate(GLuint projectionLoc, GLuint viewLoc)
{
/* TODO: sort things based on the transparency?? You have to draw the
     * "background" first so that the "transparent" part of the texture renders
     * the background properly. otherwise, you'll just get a blank background.
     */
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));
}

inline void LoadShaders(GameMetadata *gameMetadata)
{
    /* load shaders */
    gameMetadata->program = CreateProgram("materials/programs/vertex.glsl",
                                          "materials/programs/fragment.glsl");
    gameMetadata->debugProgram =
        CreateProgram("materials/programs/vertex.glsl",
                      "materials/programs/debug_fragment_shaders.glsl");
}

inline void LoadAssets(GameMetadata *gameMetadata)
{

    GameMemory *reservedMemory = &gameMetadata->reservedMemory;

    auto *awesomefaceBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));
    SetBitmap(awesomefaceBitmap, "awesomeface", TextureParam{ GL_LINEAR, GL_LINEAR },
              g_bitmapID++, "./materials/textures/awesomeface.png");
    PushBitmap(&gameMetadata->bitmapSentinelNode, awesomefaceBitmap);

    if (gameMetadata->isOpenGLActive)
    {
        gameMetadata->textureID = OpenGLBindBitmapToTexture(awesomefaceBitmap);
    }

    auto *newBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));
    SetBitmap(newBitmap, "arche", TextureParam{ GL_NEAREST, GL_NEAREST },
              g_bitmapID++, "./materials/textures/arche.png");
    PushBitmap(&gameMetadata->bitmapSentinelNode, newBitmap);

    auto *boxBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));
    SetBitmap(boxBitmap, "box", TextureParam{ GL_LINEAR, GL_LINEAR },
              g_bitmapID++, "./materials/textures/container.png");
    PushBitmap(&gameMetadata->bitmapSentinelNode, boxBitmap);

    Bitmap *archeBitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, newBitmap->bitmapID);
    ASSERT(archeBitmap != nullptr);

    gameMetadata->playerRect->bitmapID = archeBitmap->bitmapID;

    /* XXX: There's a loose coupling between frame animation and bitmap width
     * and height. We need the width and height to make a conversion to pixel
     * to UV coordinates
     */
    u32 bitmapWidth = archeBitmap->width;
    u32 bitmapHeight = archeBitmap->height;

    /* Replace with read file data */
    FrameAnimation *frameAnimations =
        (FrameAnimation *)AllocateMemory(reservedMemory, sizeof(FrameAnimation));
    ZeroSize(frameAnimations, sizeof(FrameAnimation));

    LoadFrameData(frameAnimations, "./assets/texture_data/frames.txt");
    /* TODO: free the frames if we don't need it anymore */

    AddFrameAnimationNode(&gameMetadata->frameAnimationSentinelNode, frameAnimations);

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

            Bitmap *bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, "box");
            collissionRect->bitmapID = bitmap->bitmapID;
            PushBack(&(g_rectManager->NonTraversable), collissionRect);
        }
    }
}
#endif
