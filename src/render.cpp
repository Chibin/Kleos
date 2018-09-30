#ifndef __RENDER__
#define __RENDER__

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
#define GLM_SWIZZLE
#define GLM_SWIZZLE_XYZW
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

#include "graphics/my_vulkan.h"

#define UPDATEANDRENDER(name) \
    bool name(GameMetadata *gameMetadata)
#define RENDER(name)                                                    \
    void name(GLuint vao, GLuint vbo, GLuint textureID, GLuint program, \
              GLuint debugProgram, Entity *entity)

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

void Render(GameMetadata *gameMetadata, GLuint vao, GLuint vbo, GLuint textureID, GLuint program,
            GLuint debugProgram, Entity *entity, RectDynamicArray *hitBoxes, RectDynamicArray *hurtBoxes,
            RenderGroup *perFrameRenderGroup, VulkanContext *vc);
void Update(GameMetadata *gameMetadata, GameTimestep *gameTimestep, RectDynamicArray *hitBoxes,
            RectDynamicArray *hurtBoxes, RenderGroup *perFrameRenderGroup);
void LoadStuff(GameMetadata *gameMetadata);
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
static bool g_debugMode = false;
static bool g_spriteDirectionToggle = false;
static Animation2D *g_spriteAnimation = nullptr;
static GLuint g_permanentTextureID;
static VulkanBuffers g_vkBuffers;

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
        TTF_Font *font = OpenFont();
        ASSERT(font != nullptr);
        gameMetadata->font = font;
        //ASSERT(TTF_FontFaceIsFixedWidth(font) == 1);

        g_vkBuffers.count = 0;
        g_vkBuffers.maxNum = 100;

        if (gameMetadata->isVulkanActive)
        {
            s32 texWidth = 0;
            s32 texHeight = 0;
            s32 texChannels = 0;
            stbi_uc* pixels = stbi_load("./materials/textures/awesomeface.png",
                    &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            vc->textures[0].texWidth = texWidth;
            vc->textures[0].texHeight = texHeight;
            /* stbi doesn't have a "pitch" per se, but it's probably
             * the same as the width.
             */
            vc->textures[0].texPitch = texWidth;
            vc->textures[0].dataSize = texWidth * texHeight * 4;
            vc->textures[0].data = pixels;

            bool useStagingBuffer = false;

            Bitmap stringBitmap = {};
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

            VulkanPrepareTexture(vc,
                    &vc->gpu,
                    &vc->device,
                    &vc->setupCmd,
                    &vc->cmdPool,
                    &vc->queue,
                    &vc->memoryProperties,
                    useStagingBuffer,
                    vc->textures,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            stbi_image_free(pixels);

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

        v3 cameraPos = { 0, 0, 5 };
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

        gameMetadata->whiteBitmap.width = 1;
        gameMetadata->whiteBitmap.height = 1;
        gameMetadata->whiteBitmap.format = GL_RGBA;
        gameMetadata->whiteBitmap.textureParam = TextureParam{ GL_NEAREST, GL_NEAREST };
        gameMetadata->whiteBitmap.data = (u8 *)AllocateMemory(reservedMemory, 1 * 1 * sizeof(u32));
        for (memory_index i = 0; i < 1 * 1; i++)
        {
            /* alpha -> blue -> green -> red: 1 byte each */
            *((u32 *)gameMetadata->whiteBitmap.data + i) = 0x33000000;
        }

        LoadAssets(gameMetadata);

        LoadStuff(gameMetadata);

        gameMetadata->initFromGameUpdateAndRender = true;

        //ParticleSystem *ps = &gameMetadata->particleSystem;
        f32 base = -1.01f;
        f32 width = 0.05f;
        f32 height = 0.05f;
        Bitmap *bitmap = FindBitmap(&gameMetadata->sentinelNode, 0);
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
            particle->rect.isScreenCoordinateSpace = false;
            particle->rect.bitmapID = 0;
            particle->rect.bitmap = bitmap;
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
        case SDL_MOUSEWHEEL:
            ProcessMouseInput(event, g_camera);
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

    /* TODO: One time init might be done here as the game progress ? */

    if (gameMetadata->isOpenGLActive)
    {

        if (!gameMetadata->initOpenGL)
        {
            gameMetadata->initOpenGL = true;

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
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // NOLINT
    }

    ASSERT(gameMetadata->program);
    ASSERT(gameMetadata->debugProgram);

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
    Rect nextUpdate = *gameMetadata->playerRect;
    nextUpdate.min[0] = e->position.x + e->velocity.x * dt;
    nextUpdate.max[0] =
        e->position.x + nextUpdate.width + e->velocity.x * dt;
    nextUpdate.min[1] =
        e->position.y + e->velocity.y * dt + 0.5f * e->acceleration.y * dt * dt;
    nextUpdate.max[1] =
        nextUpdate.height +
        e->position.y + e->velocity.y * dt + 0.5f * e->acceleration.y * dt * dt;

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
        printf("rect X- min: %f max: %f\n", rect->minX, rect->maxX);
        printf("rect Y- min: %f max: %f\n", rect->minY, rect->maxY);

        if (rect->type == COLLISION && TestAABBAABB(rect, &nextUpdate))
        {
            /* how to differentiate between x and y axis? */
            printf("I hit something!!!!!!!\n");
            e->position.y = rect->maxY;
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

    if (e->willAttack)
    {
        v3 startingPosition = v3{ e->position.x, e->position.y, e->position.z };
        startingPosition += v3{ -2, 0, 0 };
        f32 rectWidth = 0.35f;
        f32 rectHeight = 0.175f;
        // GenerateAttackFrameRectPosition(rect);
        Rect *attackHitBox = CreateRectangle(perFrameMemory, startingPosition, COLOR_RED, rectWidth, rectHeight);
        PushBack(hitBoxes, attackHitBox);
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
        }
        else if (e->velocity.x < 0)
        {
            gameMetadata->playerRect->frameDirection = LEFT;
        }
        /* else don't update */

        /* follow the character around */
        CameraUpdateTarget(g_camera, e->position);
        UpdatePosition(gameMetadata->playerRect, v3{ e->position.x, e->position.y, e->position.z });

        UpdateCurrentFrame(g_spriteAnimation, 17.6f);
        UpdateFrameDirection(g_spriteAnimation, gameMetadata->playerRect->frameDirection);
        UpdateUV(g_rectManager->player, *g_spriteAnimation->currentFrame);
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
        if (particle->positionOffset.y <= gameMetadata->playerRect->minY)
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

    /* Update entities */
    UpdateEntities(gameMetadata, gameTimestep, hitBoxes, hurtBoxes, perFrameRenderGroup, true);
}

void Render(GameMetadata *gameMetadata, GLuint vao, GLuint vbo, GLuint textureID, GLuint program,
            GLuint debugProgram, Entity *entity, RectDynamicArray *hitBoxes, RectDynamicArray *hurtBoxes,
            RenderGroup *perFrameRenderGroup, VulkanContext *vc)
{

    struct PushConstantMatrix
    {
        glm::mat4 view;
        glm::mat4 proj;
    } pushConstants;
    pushConstants = {};

    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;
    GameTimestep *gt = gameMetadata->gameTimestep;
    char buffer[256];
    UniformBufferObject ubo = {};

    Bitmap perFrameSentinelNode = {};
    perFrameSentinelNode.next = &perFrameSentinelNode;
    perFrameSentinelNode.prev = &perFrameSentinelNode;

    u64 endCounter = SDL_GetPerformanceCounter();
    u64 counterElapsed = endCounter - gt->lastCounter;

    f64 MSPerFrame = (((1000.0f * (real64)counterElapsed) / (real64)gt->perfCountFrequency));
    f64 FPS = (real64)gt->perfCountFrequency / (real64)counterElapsed;
    gt->lastCounter = endCounter;

    u64 endCycleCount = __rdtsc();
    u64 cyclesElapsed = endCycleCount - gt->lastCycleCount;
    f64 MCPF = ((f64)cyclesElapsed / (1000.0f * 1000.0f));

    gt->lastCycleCount = endCycleCount;

    if (gameMetadata->isVulkanActive)
    {
        VulkanPrepareRender(vc);
        VulkanBeginRenderPass(vc);
        VulkanSetViewportAndScissor(vc);
    }

    GLuint modelLoc, viewLoc, projectionLoc;

    if (gameMetadata->isOpenGLActive)
    {
        /* TODO: Fix alpha blending... it's currently not true transparency.
         * you need to disable this if you want the back parts to be shown when
         * alpha blending
         */
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        /* TODO: sort materials to their own group for that specific program
         * bind to the new program
         */
        Entity *player = nullptr;
        player = entity;

        /* programs used first will have higher priority being shown in the
         * canvas
         */
        glBindVertexArray(vao);

        /* TODO: remove this from here... this is just testing it out */
        glm::mat4 position = glm::mat4();
        position = glm::translate(position, player->position);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_OF_RECT_CORNER,
                g_player->data, GL_STATIC_DRAW);

        OpenGLCheckErrors();
        OpenGLBeginUseProgram(program, textureID);

        /* load uniform variable to shader program before drawing */
        modelLoc = glGetUniformLocation(program, "model");
        viewLoc = glGetUniformLocation(program, "view");
        projectionLoc = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(position));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

        OpenGLCheckErrors();
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
    }

    /* DRAW UI */
    if (gameMetadata->isOpenGLActive)
    {
        SetOpenGLDrawToScreenCoordinate(projectionLoc, viewLoc);
    }

    if (gameMetadata->isVulkanActive)
    {
        ubo = {};
        ubo.view = glm::mat4();
        ubo.projection = glm::mat4();
        ubo.model = glm::mat4();
        VulkanUpdateUniformBuffer(vc, &ubo);
    }

    gameMetadata->playerRect->bitmap = FindBitmap(&gameMetadata->sentinelNode,
                                                  gameMetadata->playerRect->bitmapID);
    PushRenderGroupRectInfo(perFrameRenderGroup, gameMetadata->playerRect);

    ASSERT(g_rectManager->Traversable.size > 0);
    for (memory_index i = 0; i < g_rectManager->Traversable.size; i++)
    {
        Rect *rect = g_rectManager->Traversable.rects[i];
        rect->bitmap = FindBitmap(&gameMetadata->sentinelNode, rect->bitmapID);
        ASSERT(rect->bitmap);
        PushRenderGroupRectInfo(perFrameRenderGroup, rect);
    }

    ClearUsedVertexRenderGroup(perFrameRenderGroup);

    Bitmap *bitmap = nullptr;
    Bitmap *prevBitmap = nullptr;
    TextureParam prevTextureParam = {};
    b32 isPrevScreenCoordinateSpace = false;

    for (memory_index i = 0; i < perFrameRenderGroup->rectEntityCount; i++)
    {
        Rect *rect = (Rect *)perFrameRenderGroup->rectMemory.base + i;

        //bitmap = FindBitmap(&gameMetadata->sentinelNode, rect->bitmapID);
        bitmap = rect->bitmap;
        ASSERT(bitmap != nullptr);

        TextureParam textureParam = bitmap->textureParam;

        if ((bitmap != prevBitmap) ||
            (textureParam != prevTextureParam ||
            (isPrevScreenCoordinateSpace != rect->isScreenCoordinateSpace)))
        {

            if (gameMetadata->isVulkanActive)
            {
                if (perFrameRenderGroup->vertexMemory.used > 0)
                {
                    ASSERT(perFrameRenderGroup->vertexMemory.used > 0);
                    memset(&vc->vertices, 0, sizeof(vc->vertices));
                    VulkanPrepareVertices(
                            vc,
                            &g_vkBuffers.bufs[g_vkBuffers.count],
                            &g_vkBuffers.mems[g_vkBuffers.count],
                            (void *)perFrameRenderGroup->vertexMemory.base,
                            perFrameRenderGroup->vertexMemory.used);
                    VulkanAddDrawCmd(
                            vc,
                            &g_vkBuffers.bufs[g_vkBuffers.count++],
                            SafeCastToU32(perFrameRenderGroup->rectCount * 6));
                }
            }

            if (gameMetadata->isOpenGLActive)
            {
                glBufferData(GL_ARRAY_BUFFER, perFrameRenderGroup->vertexMemory.used,
                        perFrameRenderGroup->vertexMemory.base, GL_STATIC_DRAW);
                DrawRawRectangle(perFrameRenderGroup->rectCount);

            }

            ClearUsedVertexRenderGroup(perFrameRenderGroup);

            if (rect->isScreenCoordinateSpace)
            {
                if (gameMetadata->isVulkanActive)
                {
                    pushConstants.proj = glm::mat4();
                    pushConstants.view = glm::mat4();
                    vkCmdPushConstants(
                            vc->drawCmd,
                            vc->pipelineLayout,
                            VK_SHADER_STAGE_VERTEX_BIT,
                            0,
                            sizeof(PushConstantMatrix),
                            &pushConstants);
                }
                if (gameMetadata->isOpenGLActive)
                {
                    SetOpenGLDrawToScreenCoordinate(viewLoc, projectionLoc);
                }
            }
            else
            {
                if (gameMetadata->isVulkanActive)
                {
                    pushConstants.proj = *g_projection;
                    pushConstants.view = g_camera->view;
                    vkCmdPushConstants(
                            vc->drawCmd,
                            vc->pipelineLayout,
                            VK_SHADER_STAGE_VERTEX_BIT,
                            0,
                            sizeof(PushConstantMatrix),
                            &pushConstants);
                }
                if (gameMetadata->isOpenGLActive)
                {
                    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
                    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));
                }

            }

            if (gameMetadata->isOpenGLActive)
            {
                OpenGLUpdateTextureParameter(&textureParam);

                if (bitmap != prevBitmap)
                {
                    OpenGLLoadBitmap(bitmap, textureID);
                }
            }
            prevBitmap = bitmap;
            isPrevScreenCoordinateSpace = rect->isScreenCoordinateSpace;
        }

        PushRenderGroupRectVertex(perFrameRenderGroup, rect);
        prevTextureParam = textureParam;
    }

    if (gameMetadata->isVulkanActive)
    {
        pushConstants.proj = *g_projection;
        pushConstants.view = g_camera->view;
        vkCmdPushConstants(
                vc->drawCmd,
                vc->pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(PushConstantMatrix),
                &pushConstants);

        ASSERT(perFrameRenderGroup->vertexMemory.used > 0);
        VulkanPrepareVertices(
                vc,
                &g_vkBuffers.bufs[g_vkBuffers.count],
                &g_vkBuffers.mems[g_vkBuffers.count],
                (void *)perFrameRenderGroup->vertexMemory.base,
                perFrameRenderGroup->vertexMemory.used);
        VulkanAddDrawCmd(
                vc,
                &g_vkBuffers.bufs[g_vkBuffers.count++],
                SafeCastToU32(perFrameRenderGroup->rectCount * 6));
    }

    if (gameMetadata->isOpenGLActive)
    {
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

        glBufferData(GL_ARRAY_BUFFER, perFrameRenderGroup->vertexMemory.used,
                perFrameRenderGroup->vertexMemory.base, GL_STATIC_DRAW);
        DrawRawRectangle(perFrameRenderGroup->rectCount);
    }

    ClearUsedVertexRenderGroup(perFrameRenderGroup);
    ClearUsedRectInfoRenderGroup(perFrameRenderGroup);

    Bitmap *lastBitmapBeforeUI = bitmap;

    /* DRAW UI for OpenGL */
    Bitmap stringBitmap = {};
    sprintf_s(buffer, sizeof(char) * 150, "  %.02f ms/f    %.0ff/s    %.02fcycles/f  ", MSPerFrame, FPS, MCPF); // NOLINT
    StringToBitmap(perFrameMemory, &stringBitmap, gameMetadata->font, buffer);                                  // NOLINT
    stringBitmap.textureParam = TextureParam{ GL_NEAREST,  GL_NEAREST };

    f32 screenWidth = gameMetadata->screenResolution.v[0];
    f32 screenHeight = gameMetadata->screenResolution.v[1];
    f32 scaleFactor = 3.0f;
    f32 rectWidth  = stringBitmap.width / screenWidth * scaleFactor;
    f32 rectHeight = stringBitmap.height / screenHeight * scaleFactor;

    /* This is in raw OpenGL coordinates */
    v3 startingPosition = v3{ -1, 1 - rectHeight, 0 };

    Rect *statsRect =
        CreateRectangle(perFrameMemory, startingPosition, COLOR_BLACK, rectWidth, rectHeight);
    statsRect->isScreenCoordinateSpace = true;
    statsRect->bitmap = &stringBitmap;

    if (gameMetadata->isOpenGLActive)
    {
        PushRenderGroupRectInfo(perFrameRenderGroup, statsRect);

        statsRect->bitmapID = gameMetadata->whiteBitmap.bitmapID;
        statsRect->bitmap = &gameMetadata->whiteBitmap;
        PushRenderGroupRectInfo(perFrameRenderGroup, statsRect);

        SetOpenGLDrawToScreenCoordinate(viewLoc, projectionLoc);

        for (memory_index i = 0; i < perFrameRenderGroup->rectEntityCount; i++)
        {
            Rect *rect = (Rect *)perFrameRenderGroup->rectMemory.base + i;
            bitmap = rect->bitmap;
            ASSERT(bitmap != nullptr);

            OpenGLLoadBitmap(bitmap, textureID);

            PushRenderGroupRectVertex(perFrameRenderGroup, rect);

            glBufferData(GL_ARRAY_BUFFER, perFrameRenderGroup->vertexMemory.used,
                    perFrameRenderGroup->vertexMemory.base, GL_STATIC_DRAW);
            DrawRawRectangle(perFrameRenderGroup->rectCount);

            ClearUsedVertexRenderGroup(perFrameRenderGroup);
        }

        /* End Draw UI */

        OpenGLLoadBitmap(lastBitmapBeforeUI, textureID);
        OpenGLEndUseProgram();
    }

    g_debugMode = true;
    if (g_debugMode)
    {
        perFrameRenderGroup->rectCount = 0;
        ClearMemoryUsed(&perFrameRenderGroup->vertexMemory);

        /* Draw collissions, hurtboxes and hitboxes */
        if (gameMetadata->isOpenGLActive)
        {
            OpenGLBeginUseProgram(debugProgram, textureID);
        }

        for (int i = 0; i < g_rectManager->NonTraversable.size; i++)
        {
            Rect *rect = g_rectManager->NonTraversable.rects[i];
            PushRenderGroupRectVertex(perFrameRenderGroup, rect);
        }

        for (memory_index i = 0; i < hitBoxes->size; i++)
        {
            Rect *rect = hitBoxes->rects[i];
            PushRenderGroupRectVertex(perFrameRenderGroup, rect);
        }

        for (memory_index i = 0; i < hurtBoxes->size; i++)
        {
            Rect *rect = hurtBoxes->rects[i];
            PushRenderGroupRectVertex(perFrameRenderGroup, rect);
        }

        if (gameMetadata->isVulkanActive)
        {
            ASSERT(perFrameRenderGroup->vertexMemory.used > 0);
            VulkanPrepareVertices(
                    vc,
                    &g_vkBuffers.bufs[g_vkBuffers.count],
                    &g_vkBuffers.mems[g_vkBuffers.count],
                    (void *)perFrameRenderGroup->vertexMemory.base,
                    perFrameRenderGroup->vertexMemory.used);
            VulkanAddDrawCmd(
                    vc,
                    &g_vkBuffers.bufs[g_vkBuffers.count++],
                    SafeCastToU32(perFrameRenderGroup->rectCount * 6));

        }

        if (gameMetadata->isOpenGLActive)
        {
            modelLoc = glGetUniformLocation(debugProgram, "model");
            viewLoc = glGetUniformLocation(program, "view");
            projectionLoc = glGetUniformLocation(program, "projection");

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));
            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
            glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

            glBufferData(GL_ARRAY_BUFFER, perFrameRenderGroup->vertexMemory.used,
                    perFrameRenderGroup->vertexMemory.base, GL_STATIC_DRAW);

            DrawRawPointRectangle(perFrameRenderGroup->rectCount);
            DrawRawRectangle(perFrameRenderGroup->rectCount);
            OpenGLEndUseProgram();
        }

    }

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
                NULL);

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

        ClearUsedVertexRenderGroup(perFrameRenderGroup);
        PushRenderGroupRectInfo(perFrameRenderGroup, statsRect);

        statsRect->bitmapID = gameMetadata->whiteBitmap.bitmapID;
        statsRect->bitmap = &gameMetadata->whiteBitmap;

        PushRenderGroupRectInfo(perFrameRenderGroup, statsRect);
        for (memory_index i = 0; i < perFrameRenderGroup->rectEntityCount; i++)
        {
            Rect *rect = (Rect *)perFrameRenderGroup->rectMemory.base + i;
            PushRenderGroupRectVertex(perFrameRenderGroup, rect);
        }
        VulkanPrepareVertices(
                vc,
                &g_vkBuffers.bufs[g_vkBuffers.count],
                &g_vkBuffers.mems[g_vkBuffers.count],
                (void *)perFrameRenderGroup->vertexMemory.base,
                perFrameRenderGroup->vertexMemory.used);
        VulkanAddDrawCmd(
                vc,
                &g_vkBuffers.bufs[g_vkBuffers.count++],
                SafeCastToU32(perFrameRenderGroup->rectCount * 6));

        VulkanEndBufferCommands(vc);
        VulkanEndRender(vc);
        vkFreeMemory(vc->device, vc->vertices.mem, nullptr);
        vkDestroyBuffer(vc->device, vc->vertices.buf, nullptr);

        for (memory_index i = 0; i < g_vkBuffers.count; i++)
        {
            vkDestroyBuffer(vc->device, g_vkBuffers.bufs[i], nullptr);
            vkFreeMemory(vc->device, g_vkBuffers.mems[i], nullptr);
        }
        g_vkBuffers.count = 0;
    }

    if (gameMetadata->isOpenGLActive)
    {
        glBindVertexArray(0);
        OpenGLCheckErrors();

        OpenGLEndUseProgram();
    }
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
            r->bitmapID = 0;
            PushBack(&(g_rectManager->Traversable), r);
        }
    }

    /* Let's add some non-traversable entity */
    for (int i = 0; i < 10; i++)
    {
        v3 startingPosition = { -1 + 4 * (real32)i, 0, 0 };

        Entity *collisionEntity =
            AddNewEntity(reservedMemory, g_entityManager, startingPosition);
        ASSERT(collisionEntity != nullptr);
        collisionEntity->isTraversable = false;
        collisionEntity->isPlayer = false;

        Rect *collissionRect =
            CreateRectangle(reservedMemory, startingPosition, color, 3, 5);
        AssociateEntity(collissionRect, collisionEntity, false);

        if (i == 2)
        {
            UpdateColors(collissionRect, v4{ 1.0f, 0.0f, 0.0f, 0.7f });
            collissionRect->type = HURTBOX;
        }
        else if (i == 3)
        {

            collissionRect->type = HITBOX;
            UpdateColors(collissionRect, v4{ 0.0f, 1.0f, 0.0f, 0.7f });
        }
        else
        {

            collissionRect->type = COLLISION;
            UpdateColors(collissionRect, v4{ 0.0f, 0.0f, 1.0f, 0.7f });
        }

        collissionRect->bitmapID = 0;
        PushBack(&(g_rectManager->NonTraversable), collissionRect);
    }
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
    LoadShaders(gameMetadata);

    static memory_index g_bitmapID = 0;
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;

    auto *awesomefaceBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));
    SetBitmap(awesomefaceBitmap, TextureParam{ GL_LINEAR, GL_LINEAR },
              g_bitmapID++, "./materials/textures/awesomeface.png");
    PushBitmap(&gameMetadata->sentinelNode, awesomefaceBitmap);

    gameMetadata->textureID = OpenGLBindBitmapToTexture(awesomefaceBitmap);

    auto *newBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));
    SetBitmap(newBitmap, TextureParam{ GL_NEAREST, GL_NEAREST },
              g_bitmapID++, "./materials/textures/arche.png");
    PushBitmap(&gameMetadata->sentinelNode, newBitmap);

    Bitmap *archeBitmap = FindBitmap(&gameMetadata->sentinelNode, newBitmap->bitmapID);
    ASSERT(archeBitmap != nullptr);

    gameMetadata->playerRect->bitmapID = archeBitmap->bitmapID;

    u32 bitmapWidth = archeBitmap->width;
    u32 bitmapHeight = archeBitmap->height;
    f32 spriteHeight = 60.0f;
    f32 basePixelHeight = bitmapHeight - spriteHeight;

    g_spriteAnimation = (Animation2D *)AllocateMemory(reservedMemory, sizeof(Animation2D));
    ZeroSize(g_spriteAnimation, sizeof(Animation2D));
    g_spriteAnimation->direction = LEFT;
    g_spriteAnimation->totalFrames = 2;
    g_spriteAnimation->frameCoords =
        (RectUVCoords *)AllocateMemory(reservedMemory, sizeof(RectUVCoords) * g_spriteAnimation->totalFrames);
    g_spriteAnimation->timePerFrame = 1000 * 0.75;

#if 1
    /* TODO: Need to figure out how to use compound literals with specific assignment in windows */
    v2 topRight = {}, bottomRight = {}, bottomLeft = {}, topLeft = {};
    g_spriteAnimation->frameCoords[0] = RectUVCoords{ topRight    = PixelToUV(v2{ 60, basePixelHeight + spriteHeight }, bitmapWidth, bitmapHeight),
                                                      bottomRight = PixelToUV(v2{ 60, basePixelHeight }, bitmapWidth, bitmapHeight),
                                                      bottomLeft  = PixelToUV(v2{ 0, basePixelHeight }, bitmapWidth, bitmapHeight),
                                                      topLeft     = PixelToUV(v2{ 0, basePixelHeight + spriteHeight }, bitmapWidth, bitmapHeight)};

    g_spriteAnimation->frameCoords[1] = RectUVCoords{ topRight    = PixelToUV(v2{ 60 + 60, basePixelHeight + spriteHeight }, bitmapWidth, bitmapHeight),
                                                      bottomRight = PixelToUV(v2{ 60 + 60, basePixelHeight }, bitmapWidth, bitmapHeight),
                                                      bottomLeft  = PixelToUV(v2{ 0 + 60, basePixelHeight }, bitmapWidth, bitmapHeight),
                                                      topLeft     = PixelToUV(v2{ 0 + 60, basePixelHeight + spriteHeight }, bitmapWidth, bitmapHeight)};
#else
    /* https://en.m.wikipedia.org/wiki/C_syntax#Designated_initializers */
    g_spriteAnimation->frameCoords[0] = RectUVCoords{ .topRight    = PixelToUV(v2{ 60, basePixelHeight + spriteHeight }, bitmapWidth, bitmapHeight),
                                                      .bottomRight = PixelToUV(v2{ 60, basePixelHeight }, bitmapWidth, bitmapHeight),
                                                      .bottomLeft  = PixelToUV(v2{ 0, basePixelHeight }, bitmapWidth, bitmapHeight),
                                                      .topLeft     = PixelToUV(v2{ 0, basePixelHeight + spriteHeight }, bitmapWidth, bitmapHeight)};

    g_spriteAnimation->frameCoords[1] = RectUVCoords{ .topRight    = PixelToUV(v2{ 60 + 60, basePixelHeight + spriteHeight }, bitmapWidth, bitmapHeight),
                                                      .bottomRight = PixelToUV(v2{ 60 + 60, basePixelHeight }, bitmapWidth, bitmapHeight),
                                                      .bottomLeft  = PixelToUV(v2{ 0 + 60, basePixelHeight }, bitmapWidth, bitmapHeight),
                                                      .topLeft     = PixelToUV(v2{ 0 + 60, basePixelHeight + spriteHeight }, bitmapWidth, bitmapHeight)};
#endif

}
#endif
