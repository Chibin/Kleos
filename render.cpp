#ifndef __RENDER__
#define __RENDER__

#include <stddef.h> /* offsetof */
#include <stdlib.h> /* abs */
#include <string>
#include <stdio.h>

#include "sdl_common.h"

#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "logger.h"
#include "math.h"
#pragma warning(pop)

#include "game_memory.h"
static GameMemory *g_reservedMemory = NULL;
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
#include "stb_image.h"
#pragma warning(pop)
#endif

/* probably need to be here? depends where we put our game logic */
#include "font.cpp"
#include "camera.cpp"
#include "entity.cpp"
#include "entity_manager.cpp"
#include "game_time.cpp"
#include "input.cpp"
#include "opengl.cpp"

#include "collision.cpp"
#include "rect_manager.cpp"
#include "rectangle.cpp"

#define UPDATEANDRENDER(name)                                                  \
    bool name(GLuint vao, GLuint vbo, GLuint textureID, GLuint program,        \
              GLuint debugProgram, v2 screenResolution,                        \
              GameMetadata *gameMetadata)
#define RENDER(name)                                                           \
    void name(GLuint vao, GLuint vbo, GLuint textureID, GLuint program,        \
              GLuint debugProgram, Entity *entity)

#define COLOR_WHITE v4{1, 1, 1, 1}
#define COLOR_RED v4{1, 0, 0, 1}
#define COLOR_GREEN v4{0, 1, 0, 1}
#define COLOR_BLUE v4{0, 0, 1, 1}
#define COLOR_BLACK v4{0, 0, 0, 1}

void Render(GameMetadata *gameMetadata, GLuint vao, GLuint vbo, GLuint textureID, GLuint program,
            GLuint debugProgram, Entity *entity);
void RenderAllEntities(GLuint program);
void Update(GameMetadata *gameMetadata, Entity *player, GameTimestep *gameTimestep);
void LoadStuff(GameMetadata *gameMetadata);
inline void LoadAssets(GameMetadata *gameMetadata);

/* TODO: We'll need to get rid of these global variables later on */
Camera *g_camera = NULL;
glm::mat4 *g_projection = NULL;
GLfloat *g_rectangleVertices = NULL;
static bool init = false;
EntityManager *g_entityManager = NULL;
Entity *g_player = NULL;
Rect *g_playerRect = NULL;
RectManager *g_rectManager = NULL;
EntityDynamicArray *g_eda = NULL;
static bool g_debugMode = false;
static bool g_spriteDirectionToggle = false;
static Animation2D *g_spriteAnimation = NULL;

extern "C" UPDATEANDRENDER(UpdateAndRender)
{

    SDL_Event event;
    bool continueRunning = true;

    GameTimestep **gameTimestep = &gameMetadata->gameTimestep;
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;
    GameMemory *perFrameMemory = &gameMetadata->transientMemory;
    ClearMemoryUsed(perFrameMemory);

    if (!init)
    {

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

        v3 cameraPos = {0, 0, 5};
        // and looks at the origin
        v3 cameraTarget = {0, 0, 0};
        // Head is up (set to 0,-1,0 to look upside-down)
        v3 cameraUp = {0, 1, 0};
        g_camera = CreateCamera(reservedMemory, cameraPos, cameraTarget, cameraUp);

        g_eda = CreateEntityDynamicArray(reservedMemory);

        float screen_width = screenResolution.v[0];
        float screen_height = screenResolution.v[1];
        g_projection = (glm::mat4 *)AllocateMemory(reservedMemory, (sizeof(glm::mat4)));
        *g_projection =
            glm::infinitePerspective(45.0f, screen_width / screen_height, 0.1f);

        g_rectManager = CreateRectManager(reservedMemory);

        /* TODO: May be the entity manager should be the only one creating the
         * entities?
         */
        g_entityManager = CreateEntityManger(reservedMemory);

        Entity newPlayer;
        newPlayer.position = glm::vec3(0, 0, 0);
        newPlayer.isPlayer = true;

        int index = Append(reservedMemory, g_entityManager, &newPlayer);

        /* NOTE: we don't need to free the player since we created it in the
         * stack
         */
        v3 pos = { 0, 0, 0.01f };
        g_player = &g_entityManager->entities[index];
        g_player->type = 2;
        g_playerRect = CreateRectangle(reservedMemory, pos, COLOR_WHITE, 2, 1);
        AssociateEntity(g_playerRect, g_player, false);
        g_rectManager->player = g_playerRect;
        g_playerRect->type = REGULAR;
        g_playerRect->frameDirection = LEFT;

        gameMetadata->whiteBitmap.width = 1;
        gameMetadata->whiteBitmap.height = 1;
        gameMetadata->whiteBitmap.format = GL_RGBA;
        gameMetadata->whiteBitmap.data = (u8 *)AllocateMemory(reservedMemory, 1 * 1 * sizeof(u32));
        for(memory_index i =0; i < 1 * 1; i++)
        {
            /* alpha -> blue -> green -> red: 1 byte each */
            *((u32 *)gameMetadata->whiteBitmap.data + i) = 0x33000000;
        }

        LoadAssets(gameMetadata);
        LoadStuff(gameMetadata);
        init = true;
    }

    /* NOTE: Looks very player centric right now, not sure if we need to make it
     * better later on.
     * Use keystate to get the feel for the fastest response time.
     * Keyrepeat is doesn't feel smooth enough for us to use it as a way to
     * move. The amount of keyboard repeat depends on the setting the user has
     * on their computer, so not reliable.
     */
    const Uint8 *keystate = SDL_GetKeyboardState(NULL);
    ProcessKeysHeldDown(g_player, keystate);

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            return false;

        if (event.type == SDL_MOUSEWHEEL)
            ProcessMouseInput(event, g_camera);

        else if (event.type == SDL_KEYDOWN)
            ProcessInputDown(event.key.keysym.sym, &continueRunning);

        else if (event.type == SDL_KEYUP)
            ProcessInputUp(event.key.keysym.sym);
    }

    /* TODO: One time init might be done here as the game progress ? */

    /* start with a 'clear' screen */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Update(gameMetadata, g_player, *gameTimestep);
    Render(gameMetadata, vao, vbo, textureID, program, debugProgram, g_player);

    return continueRunning;
}

void UpdateEntities(GameMetadata *gameMetadata, Entity *e, GameTimestep *gt, bool isPlayer = false)
{
    //GameMemory *reservedMemory = &gameMetadata->reservedMemory;
    GameMemory *perFrameMemory = &gameMetadata->transientMemory;

    /* NOTE: Look at this later Axis-Aligned Bounding Box*/
    // position, velocity, acceleration
    const GLfloat gravity = -9.81f;

    RectDynamicArray *hitBoxes = CreateRectDynamicArray(perFrameMemory, 100);
    RectDynamicArray *hurtBoxes = CreateRectDynamicArray(perFrameMemory, 100);
    f32 dt = gt->dt;

    //      if (IntersectionAABB(rect, v2{e->position.x, e->position.y},
    //      rayDirection))
    Rect nextUpdate = *g_playerRect;
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

    for (int i = 0; i < hitBoxes->size; i++)
    {
        Rect *rect = hitBoxes->rects[i];
        for (int y = 0; y < hurtBoxes->size; y++)
        {
            Rect hb = *(hurtBoxes->rects[i]);
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
        if ( e->velocity.x > 0 )
        {
            g_playerRect->frameDirection = RIGHT;
        }
        else if ( e->velocity.x < 0)
        {
            g_playerRect->frameDirection = LEFT;
        }
        /* else don't update */


        /* follow the character around */
        CameraUpdateTarget(g_camera, e->position);
        UpdatePosition(g_playerRect, v3{e->position.x, e->position.y, e->position.z});

        UpdateCurrentFrame(g_spriteAnimation, 17.6f);
        UpdateFrameDirection(g_spriteAnimation, g_playerRect->frameDirection);
        UpdateUV(g_rectManager->player, *g_spriteAnimation->currentFrame);
    }

    /* Apply "friction" */
    e->velocity.x = 0;


}

void Update(GameMetadata *gameMetadata, Entity *player, GameTimestep *gameTimestep)
{
    /* update logics and data here */
    /* physics */
    UpdateGameTimestep(gameTimestep);

    /* Update entities */
    UpdateEntities(gameMetadata, player, gameTimestep, true);
}

void RenderAllEntities(GLuint vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void Render(GameMetadata *gameMetadata, GLuint vao, GLuint vbo, GLuint textureID, GLuint program,
            GLuint debugProgram, Entity *entity)
{
    GameMemory *perFrameMemory = &gameMetadata->transientMemory;
    GameTimestep *gt = gameMetadata->gameTimestep;
    char buffer[150];
    u64 endCounter = SDL_GetPerformanceCounter();
    u64 counterElapsed = endCounter - gt->lastCounter;

    f64 MSPerFrame = (((1000.0f * (real64)counterElapsed) / (real64)gt->perfCountFrequency));
    f64 FPS = (real64)gt->perfCountFrequency / (real64)counterElapsed;
    gt->lastCounter = endCounter;

    u64 endCycleCount = __rdtsc();
    u64 cyclesElapsed = endCycleCount - gt->lastCycleCount;
    f64 MCPF = ((f64)cyclesElapsed / (1000.0f * 1000.0f));

    gt->lastCycleCount = endCycleCount;

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
    Entity *player = NULL;
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

    GLuint modelLoc, viewLoc, projectionLoc, rectTypeLoc;

    if (g_debugMode)
    {
        // DrawHurtBoxes();
        // DrawHitBoxes();
        // DrawCollisions();
    }

    const u32 numOfPointsPerRect = 6;
    const u16 maxVertexCount = 10001;
    RenderGroup renderGroup = {};
    u32 vertexBlockSize = sizeof(Vertex) * numOfPointsPerRect * maxVertexCount;
    renderGroup.vertexMemory.base = (u8 *)AllocateMemory(perFrameMemory, vertexBlockSize);
    renderGroup.vertexMemory.maxSize = vertexBlockSize;

    /* Draw text at the corner */

    GLuint temp = StringToTexture(gameMetadata->font, "testing this");
    OpenGLBindTexture(temp);
    /* End draw text at corner */

    OpenGLBeginUseProgram(program, textureID);

    /* load uniform variable to shader program before drawing */
    modelLoc = glGetUniformLocation(program, "model");
    viewLoc = glGetUniformLocation(program, "view");
    projectionLoc = glGetUniformLocation(program, "projection");
    rectTypeLoc = glGetUniformLocation(program, "type");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(position));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

    OpenGLCheckErrors();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    
    /* DRAW UI */

    /* TODO: sort things based on the transparency?? You have to draw the
     * "background" first so that the "transparent" part of the texture renders
     * the background properly. otherwise, you'll just get a blank background.
     */
#if 0
    f32  left   = 0 - g_player->position.x;
    f32  right  = 10 - g_player->position.x;
    f32  bottom = -10 - g_player->position.y;
    f32  top    = 0.8f - g_player->position.y;
    f32  near   = 0.1f;
    f32  far    = 100.0f;
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(glm::ortho(left, right, bottom, top, near, far)));
#else
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));
#endif

    Bitmap stringBitmap = {};
    sprintf_s(buffer, sizeof(char) * 150, "  %.02f ms/f    %.0ff/s    %.02fcycles/f  ", MSPerFrame, FPS, MCPF);
    StringToBitmap(perFrameMemory, &stringBitmap, gameMetadata->font, buffer);

    f32 rectWidth = 0.35f;
    f32 rectHeight = 0.175f;
    f32 padding = rectHeight * 0.1f;

    /* This is in raw OpenGL coordinates */
    v3 startingPosition = v3{-1, 1 - rectHeight + padding, 0};
    Rect *statsRect =
        CreateRectangle(perFrameMemory, startingPosition, COLOR_WHITE, rectWidth, rectHeight);

    PushRect(&renderGroup, statsRect);
    glBufferData(GL_ARRAY_BUFFER, renderGroup.vertexMemory.used,
            renderGroup.vertexMemory.base, GL_STATIC_DRAW);

    OpenGLLoadBitmap(&stringBitmap, textureID);
    DrawRawRectangle(renderGroup.rectCount);

    OpenGLLoadBitmap(&gameMetadata->whiteBitmap, textureID);
    DrawRawRectangle(renderGroup.rectCount);
    /* END DRAW UI */

    renderGroup.rectCount = 0;
    ClearMemoryUsed(&renderGroup.vertexMemory);

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

    /* Save PLAYER vertices */
    PushRect(&renderGroup, g_rectManager->player);
    glBufferData(GL_ARRAY_BUFFER, renderGroup.vertexMemory.used,
            renderGroup.vertexMemory.base, GL_STATIC_DRAW);
    Bitmap *archeBitmap = FindBitmap(&gameMetadata->sentinelNode, 1);
    OpenGLLoadBitmap(archeBitmap, textureID);

    OpenGLUpdateTextureParameter(&archeBitmap->textureParam);

    DrawRawRectangle(renderGroup.rectCount);

    renderGroup.rectCount = 0;
    ClearMemoryUsed(&renderGroup.vertexMemory);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    memory_index prevBitmapID = 1;
    Bitmap *bitmap = nullptr;
    for (memory_index i = 0; i < g_rectManager->Traversable.size; i++)
    {
        Rect *rect = g_rectManager->Traversable.rects[i];

        memory_index bitmapID = rect->bitmapID; /* GetBitmap(bitmapID);*/
        if (bitmapID != prevBitmapID)
        {
            glBufferData(GL_ARRAY_BUFFER, renderGroup.vertexMemory.used,
                    renderGroup.vertexMemory.base, GL_STATIC_DRAW);
            DrawRawRectangle(renderGroup.rectCount);

            renderGroup.rectCount = 0;
            ClearMemoryUsed(&renderGroup.vertexMemory);

            bitmap = gameMetadata->bitmaps[bitmapID];
            ASSERT(bitmap);
            /* override the bitmap on this textureID */
            OpenGLLoadBitmap(bitmap, textureID);
        }

        PushRect(&renderGroup, rect);
        prevBitmapID = bitmapID;
    }

    /* TODO: need to do a more efficient way of doing this. there's a chance
     * that an upload and draw to happen twice
     */
    glBufferData(GL_ARRAY_BUFFER, renderGroup.vertexMemory.used,
            renderGroup.vertexMemory.base, GL_STATIC_DRAW);
    DrawRawRectangle(renderGroup.rectCount);

    OpenGLEndUseProgram();

    g_debugMode = true;
    if (g_debugMode)
    {
        renderGroup.rectCount = 0;
        ClearMemoryUsed(&renderGroup.vertexMemory);

        /* Draw collissions, hurtboxes and hitboxes */
        OpenGLBeginUseProgram(debugProgram, textureID);

        modelLoc = glGetUniformLocation(debugProgram, "model");
        viewLoc = glGetUniformLocation(program, "view");
        projectionLoc = glGetUniformLocation(program, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE,
                           glm::value_ptr(g_camera->view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE,
                           glm::value_ptr(*g_projection));

        for (int i = 0; i < g_rectManager->NonTraversable.size; i++)
        {
            Rect *rect = g_rectManager->NonTraversable.rects[i];
            PushRect(&renderGroup, rect);
        }

        glBufferData(GL_ARRAY_BUFFER, renderGroup.vertexMemory.used,
                     renderGroup.vertexMemory.base, GL_STATIC_DRAW);

        DrawRawPointRectangle(renderGroup.rectCount);
        DrawRawRectangle(renderGroup.rectCount);

        OpenGLEndUseProgram();
    }

    glBindVertexArray(0);

    OpenGLCheckErrors();
}

void LoadStuff(GameMetadata *gameMetadata)
{
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;

    v4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
    /* load random data */
    for (int i = 0; i < 100; i++)
    {
        for (int y = 0; y < 100; y++)
        {
            v3 startingPosition = { (real32)i, (real32)y, 0 };
            /* TODO: extract out creating new entity from the manager */
            Entity *rectEntity =
                AddNewEntity(reservedMemory, g_entityManager, startingPosition);
            ASSERT(rectEntity != NULL);
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
    for (int i = 0; i < 5; i++)
    {
        v3 startingPosition = { -3 + 4 * (real32)i, -10, 0 };

        Entity *collisionEntity =
            AddNewEntity(reservedMemory, g_entityManager, startingPosition);
        ASSERT(collisionEntity != NULL);
        collisionEntity->isTraversable = false;
        collisionEntity->isPlayer = false;

        Rect *collissionRect =
            CreateRectangle(reservedMemory, startingPosition, color, 3, 5);
        AssociateEntity(collissionRect, collisionEntity, false);

        if (i == 2)
        {
            UpdateColors(collissionRect, v4{1.0f, 0.0f, 0.0f, 0.7f});
            collissionRect->type = HURTBOX;
        }
        else if (i == 3) {

            collissionRect->type = HITBOX;
            UpdateColors(collissionRect, v4{0.0f, 1.0f, 0.0f, 0.7f});
        }
        else
        {

            collissionRect->type = COLLISION;
            UpdateColors(collissionRect, v4{0.0f, 0.0f, 1.0f, 0.7f});
        }

        collissionRect->bitmapID = 0;
        PushBack(&(g_rectManager->NonTraversable), collissionRect);
    }
}

inline void LoadAssets(GameMetadata *gameMetadata)
{
    static memory_index g_bitmapID = 1;
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;
    Bitmap *newBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));
    ZeroSize(newBitmap, sizeof(Bitmap));

    ImageToBitmap(newBitmap, "./materials/textures/arche.png");
    newBitmap->textureParam.magFilter = GL_NEAREST;
    newBitmap->textureParam.minFilter = GL_NEAREST;
    newBitmap->bitmapID = g_bitmapID++;
    PushBitmap(&gameMetadata->sentinelNode, newBitmap);

    Bitmap *archeBitmap = FindBitmap(&gameMetadata->sentinelNode, 1);
    ASSERT(archeBitmap != nullptr);

    u32 bitmapWidth = archeBitmap->width;
    u32 bitmapHeight = archeBitmap->height;

    f32 spriteHeight = 60.0f;
    f32 basePixelHeight = bitmapHeight - spriteHeight;

    v2 topRight = PixelToUV(v2{60, basePixelHeight + spriteHeight}, bitmapWidth, bitmapHeight);
    v2 bottomRight = PixelToUV(v2{60, basePixelHeight}, bitmapWidth, bitmapHeight);
    v2 bottomLeft = PixelToUV(v2{0, basePixelHeight}, bitmapWidth, bitmapHeight);
    v2 topLeft = PixelToUV(v2{0, basePixelHeight + spriteHeight}, bitmapWidth, bitmapHeight);

    g_spriteAnimation = (Animation2D *)AllocateMemory(reservedMemory, sizeof(Animation2D));
    ZeroSize(g_spriteAnimation, sizeof(Animation2D));
    g_spriteAnimation->direction = LEFT;
    g_spriteAnimation->totalFrames = 2;
    g_spriteAnimation->frameCoords = 
        (RectUVCoords *)AllocateMemory(reservedMemory, sizeof(RectUVCoords) * g_spriteAnimation->totalFrames);
    g_spriteAnimation->timePerFrame = 1000 * 0.75;

    g_spriteAnimation->frameCoords[0].topRight = topRight;
    g_spriteAnimation->frameCoords[0].bottomRight = bottomRight;
    g_spriteAnimation->frameCoords[0].bottomLeft = bottomLeft;
    g_spriteAnimation->frameCoords[0].topLeft = topLeft;

    topRight = PixelToUV(v2{60 + 60, basePixelHeight + spriteHeight}, bitmapWidth, bitmapHeight);
    bottomRight = PixelToUV(v2{60 + 60, basePixelHeight}, bitmapWidth, bitmapHeight);
    bottomLeft = PixelToUV(v2{0 + 60, basePixelHeight}, bitmapWidth, bitmapHeight);
    topLeft = PixelToUV(v2{0 + 60, basePixelHeight + spriteHeight}, bitmapWidth, bitmapHeight);
    g_spriteAnimation->frameCoords[1].topRight = topRight;
    g_spriteAnimation->frameCoords[1].bottomRight = bottomRight;
    g_spriteAnimation->frameCoords[1].bottomLeft = bottomLeft;
    g_spriteAnimation->frameCoords[1].topLeft = topLeft;
}
#endif
