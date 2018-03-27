#ifndef __RENDER__
#define __RENDER_

#include <stddef.h> /* offsetof */
#include <stdio.h>
#include <string>
#include <stdlib.h> /* abs */

#include <SDL.h>
#include <SDL_ttf.h>
#define ASSERT(condition) SDL_assert(condition)

#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>

#pragma warning(push)
#pragma warning (disable: 4201)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "math.cpp"
#include "logger.cpp"
#pragma warning(pop)

/* probably need to be here? depends where we put our game logic */
#include "opengl.cpp"
#include "game_time.cpp"
#include "entity.cpp"
#include "entity_manager.cpp"
#include "camera.cpp"
#include "input.cpp"

#include "rectangle.cpp"
#include "rect_manager.cpp"
#include "collision.cpp"

#define UPDATEANDRENDER(name) bool name(GLuint vao, GLuint vbo, GLuint textureID, GLuint program, GLuint debugProgram, v2 screenResolution, GameTimestep **gameTimestep)
#define RENDER(name) void name(GLuint vao, GLuint vbo, GLuint textureID, GLuint program, GLuint debugProgram, Entity *entity)

void Render(GLuint vao, GLuint vbo, GLuint textureID, GLuint program, GLuint debugProgram, Entity *entity);
void RenderAllEntities(GLuint program);
void Update(Entity *player, GameTimestep *gameTimestep);
void LoadStuff();

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

extern "C" UPDATEANDRENDER(UpdateAndRender)
{

    SDL_Event event;
    bool continueRunning = true;

    if (!*gameTimestep) {
        *gameTimestep = (GameTimestep*)malloc(sizeof(GameTimestep));
        ResetGameTimestep(*gameTimestep);
    }

    if (!g_camera) {
        g_camera = CreateCamera();
    }

    if (!g_eda) {
        g_eda = CreateEntityDynamicArray();
    }

    if (!g_projection) {
        float SCREEN_WIDTH = screenResolution.v[0];
        float SCREEN_HEIGHT = screenResolution.v[1];
        g_projection = (glm::mat4*)malloc(sizeof(glm::mat4));
        *g_projection = glm::infinitePerspective(45.0f, SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f);
    }

    if (!g_rectManager) {
        g_rectManager = CreateRectManager();
    }

    if (!g_entityManager) {
        /* TODO: May be the entity manager should be the only one creating the
         * entities?
         */
        g_entityManager = CreateEntityManger();

        Entity newPlayer;
        newPlayer.position = glm::vec3(0,0,0);
        newPlayer.isPlayer = true;

        int index = Append(g_entityManager, &newPlayer);

        /* NOTE: we don't need to free the player since we created it in the
         * stack
         */
        v4 color = {0.4f, 0.0f, 0.4f, 1.0f};
        v3 pos = {0, 0, 0.01f};
        g_player = &g_entityManager->entities[index];
        g_player->type = 2;
        g_playerRect = CreateRectangle(g_player, pos, color, 2, 1);
        g_rectManager->player = g_playerRect;
    }

    if (!init) {
        LoadStuff();
        init = true;
    }

    /* NOTE: Looks very player centric right now, not sure if we need to make it
     * better later on.
     * Use keystate to get the feel for the fastest response time.
     * Keyrepeat is doesn't feel smooth enough for us to use it as a way to
     * move. The amount of keyboard repeat depends on the setting the user has
     * on their computer, so not reliable.
     */
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    ProcessKeysHeldDown(g_player, keystate);

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            return false;

        if (event.type == SDL_MOUSEWHEEL)
            ProcessMouseInput(event, g_camera);

        if (event.type == SDL_KEYDOWN)
            ProcessInputDown(event.key.keysym.sym, &continueRunning);

        if (event.type == SDL_KEYUP)
            ProcessInputUp(event.key.keysym.sym);
    }

    /* TODO: One time init might be done here as the game progress ? */

    /* start with a 'clear' screen */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Update(g_player, *gameTimestep);

    /* create scene */
    /* filter scene */
    /* render scene */

    Render(vao, vbo, textureID, program, debugProgram, g_player);
    return continueRunning;
}

void UpdateEntityPosition(Entity *e, GameTimestep *gt, bool isPlayer = false)
{
    /* NOTE: Look at this later Axis-Aligned Bounding Box*/
    // position, velocity, acceleration
    const GLfloat gravity = -4.81f;

    for (int i = 0; i < g_rectManager->NonTraversable.size; i++) {
        Rect *rect = g_rectManager->NonTraversable.rects[i];

        real32 dX = e->velocity.x;
        real32 dY =
            e->velocity.y +
            (gravity + e->acceleration.y) * gt->dt;

        glm::vec3 rayDirection = {dX, dY, 0};
        rayDirection = glm::normalize(rayDirection);
        rayDirection = glm::vec3(1/rayDirection.x, 1/rayDirection.y, 0);

        printf("rect X- min: %f max: %f\n", rect->minX, rect->maxX);
        printf("rect Y- min: %f max: %f\n", rect->minY, rect->maxY);

        g_playerRect->min[0] = e->position.x;
        g_playerRect->max[0] = e->position.x + g_playerRect->width;

        g_playerRect->min[1] = e->position.y;
        g_playerRect->max[1] = e->position.y + g_playerRect->height;

//      if (IntersectionAABB(rect, v2{e->position.x, e->position.y}, rayDirection))
        Rect nextUpdate = *g_playerRect;
        nextUpdate.min[0] = e->position.x + e->velocity.x;
        nextUpdate.max[0] = e->position.x + nextUpdate.width + e->velocity.x;
        nextUpdate.min[1] = e->position.y + (e->velocity.y + (gravity + e->acceleration.y) * gt->dt) * gt->dt;
        nextUpdate.max[1] = e->position.y + nextUpdate.height + (e->velocity.y + (gravity + e->acceleration.y) * gt->dt) * gt->dt;

        if (TestAABBAABB(rect, &nextUpdate)) {
            /* how to differentiate between x and y axis? */
            printf("I hit something!!!!!!!\n");
            e->position.y = rect->entity->position.y + rect->height;
            e->velocity.y = 0;
            e->acceleration.y = 0;
            e->position.x += e->velocity.x * gt->deltaTime;
        }
        else {
            e->acceleration.y += gravity;
            e->velocity.y += e->acceleration.y * gt->dt;
            e->position.y += e->velocity.y * gt->dt;
            e->position.x += e->velocity.x * gt->deltaTime;
        }

    }

    /* Apply "friction" */
    e->velocity.x = 0;

    if (isPlayer) {
        /* TODO: bound checking for the camera such that we only move the camera
         * when necessary
         */

        /* follow the character around */
        CameraUpdateTarget(g_camera, e->position);
    }

    //DeleteEntityDynamicArray(g_eda);
}

void Update(Entity *player, GameTimestep *gameTimestep)
{
    /* update logics and data here */
    /* physics */
    UpdateGameTimestep(gameTimestep);

    /* Update movable entities */
    UpdateEntityPosition(player, gameTimestep, true);
}

void Render(GLuint vao, GLuint vbo, GLuint textureID, GLuint program,
            GLuint debugProgram, Entity *entity)
{

    /* TODO: Fix alpha blending... it's currently not true transparency.
     * you need to disable this if you want the back parts to be shown when
     * alpha blending
     */
    glDisable(GL_CULL_FACE);
    glEnable (GL_BLEND);
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_OF_RECT_CORNER, g_player->data, GL_STATIC_DRAW);

    OpenGLCheckErrors();

    GLuint modelLoc, viewLoc, projectionLoc;

    if (g_debugMode) {
        //DrawHurtBoxes();
        //DrawHitBoxes();
        //DrawCollisions();
    }

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
    /* TODO: sort things based on the transparency?? You have to draw the
     * "background" first so that the "transparent" part of the texture renders the
     * background properly. otherwise, you'll just get a blank background.
     */

    int sizeOfVertices = sizeof(GLfloat) * 3;
    for(int i = 0; i < g_rectManager->Traversable.size; i++) {
        Entity *debugEntity = g_rectManager->Traversable.rects[i]->entity;
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(), debugEntity->position)));

        /* only load once since they're all similar -- just different positions.*/
        if (i == 0) {
            /* If the data is interleaved, you have to do multiple
             * glBufferSubData calls in order to populate all the right parts */
            /* cast to Vertex* so that we can get the proper pointer * arithmetic */
            glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)(offsetof(Vertex, position)), sizeOfVertices, debugEntity->data);
            glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)((Vertex*)offsetof(Vertex, position) + 1) , sizeOfVertices, (Vertex*)debugEntity->data + 1);
            glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)((Vertex*)offsetof(Vertex, position) + 2) , sizeOfVertices, (Vertex*)debugEntity->data + 2);
            glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)((Vertex*)offsetof(Vertex, position) + 3) , sizeOfVertices, (Vertex*)debugEntity->data + 3);
        }

        DrawRectangle();
    }

    /* Draw the player */
    Entity *entityToDraw = g_rectManager->player->entity;
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(), entityToDraw->position)));
    glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)(offsetof(Vertex, position)), sizeOfVertices, entityToDraw->data);
    glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)((Vertex*)offsetof(Vertex, position) + 1), sizeOfVertices, (Vertex*)entityToDraw->data + 1);
    glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)((Vertex*)offsetof(Vertex, position) + 2), sizeOfVertices, (Vertex*)entityToDraw->data + 2);
    glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)((Vertex*)offsetof(Vertex, position) + 3), sizeOfVertices, (Vertex*)entityToDraw->data + 3);
    DrawRectangle();

    OpenGLEndUseProgram();

#define DEBUG_SHADER 1
#if DEBUG_SHADER
    OpenGLBeginUseProgram(debugProgram, textureID);

    modelLoc = glGetUniformLocation(debugProgram, "model");
    viewLoc = glGetUniformLocation(program, "view");
    projectionLoc = glGetUniformLocation(program, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4()));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

    for( int i = 0; i < g_rectManager->NonTraversable.size; i++) {
        Entity *debugEntity = g_rectManager->NonTraversable.rects[i]->entity;

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(), debugEntity->position)));
        glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)(offsetof(Vertex, position)), sizeOfVertices, debugEntity->data);
        glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)((Vertex*)offsetof(Vertex, position) + 1) , sizeOfVertices, (Vertex*)debugEntity->data + 1);
        glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)((Vertex*)offsetof(Vertex, position) + 2) , sizeOfVertices, (Vertex*)debugEntity->data + 2);
        glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)((Vertex*)offsetof(Vertex, position) + 3) , sizeOfVertices, (Vertex*)debugEntity->data + 3);

        DrawPointRectangle();
        DrawRectangle();
    }

    OpenGLEndUseProgram();
#endif

    glBindVertexArray(0);

    OpenGLCheckErrors();
}

void RenderAllEntities(GLuint vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void LoadStuff()
{
    v4 color = {0.4f, 0.0f, 0.4f, 1.0f};
    /* load random data */
    for(int i = 0; i < 100; i++) {
        for(int y = 0; y < 100; y++) {
            v3 startingPosition = {(real32)i, (real32)y, 0};
            /* TODO: extract out creating new entity from the manager */
            Entity* rectEntity = AddNewEntity(g_entityManager,
                    startingPosition);
            ASSERT(rectEntity != NULL);
            Rect *r = CreateRectangle(rectEntity, startingPosition, color, 1, 1);
            rectEntity->isTraversable = true;
            rectEntity->isPlayer = false;
            rectEntity->type = 0;
            PushBack(&(g_rectManager->Traversable), r);
        }
    }

    /* Let's add a non-traversable entity */
    for (int i = 0; i < 2; i++) {
        v3 startingPosition = {-3 + 4* (real32)i, -10, 0};
        Entity* collisionEntity = AddNewEntity(g_entityManager, startingPosition);
        ASSERT(collisionEntity != NULL);
        Rect *collissionRect = CreateRectangle(collisionEntity, startingPosition, color, 3, 5);
        collisionEntity->isTraversable = false;
        collisionEntity->isPlayer = false;
        collisionEntity->type = 1;
        PushBack(&(g_rectManager->NonTraversable), collissionRect);
    }
}
#endif
