#ifndef __RENDER__
#define __RENDER_

#include <stddef.h> /* offsetof */
#include <stdio.h>
#include <string>

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

#define UPDATEANDRENDER(name) bool name(GLuint vao, GLuint vbo, GLuint textureID, GLuint program, GLuint debugProgram, v2 screenResolution, Vertex *vertices, GameTimestep **gameTimestep)
#define RENDER(name) void name(GLuint vao, GLuint vbo, GLuint textureID, GLuint program, GLuint debugProgram, Entity *entity)

void Render(GLuint vao, GLuint vbo, GLuint textureID, GLuint program, GLuint debugProgram, Entity *entity, Vertex *vertices);
void RenderAllEntities(GLuint program);
void Update(Entity *player, GameTimestep *gameTimestep);
void LoadStuff();

/* TODO: We'll need to get rid of these global variables later on */
Rect *g_testRectangle = NULL;
Camera *g_camera = NULL;
glm::mat4 *g_projection = NULL;
GLfloat *g_rectangleVertices = NULL;
static bool init = false;
EntityManager *g_entityManager = NULL;
Entity *g_player = NULL;

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

    if (!g_projection) {
        float SCREEN_WIDTH = screenResolution.v[0];
        float SCREEN_HEIGHT = screenResolution.v[1];
        g_projection = (glm::mat4*)malloc(sizeof(glm::mat4));
        *g_projection = glm::infinitePerspective(45.0f, SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f);
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
        v3 pos = {0, 0, 0};
        g_player = &g_entityManager->entities[index];
        CreateRectangle(g_player, pos, color, 2, 1);
    }

    if (!g_testRectangle) {
        v3 startingPosition = {-5, -5, 0};
        v4 color = {0.4f, 0.0f, 0.4f, 1.0f};
        Entity* rectEntity = AddNewEntity(g_entityManager);
        rectEntity->isPlayer = false;
        g_testRectangle = CreateRectangle(rectEntity, startingPosition, color, 1, 1);
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
    Render(vao, vbo, textureID, program, debugProgram, g_player, vertices);
    return continueRunning;
}

void UpdateEntityPosition(Entity *e, GameTimestep *gt, bool isPlayer = false)
{
    /* NOTE: Look at this later Axis-Aligned Bounding Box*/
    uint32 numOfNTs = 0;

    EntityDynamicArray *eda = CreateEntityDynamicArray();
    GetNonTraversableEntities(eda,
            g_entityManager->entities,
            g_entityManager->size);

    bool canUpdatePosition = true;
    for(uint32 i = 0; i < numOfNTs && eda != NULL; i++)
    {
        canUpdatePosition = false;
    }

    e->position.x += e->velocity.x * gt->deltaTime;

    /* Apply "friction" */
    e->velocity.x = 0;

    if (canUpdatePosition) {
        /* did entity hit floor */
        if (e->position.y < -1) {
            e->position.y = -1;
            e->velocity.y = 0;
            e->acceleration.y = 0;
        }
    }

    if (isPlayer) {
        /* TODO: bound checking for the camera such that we only move the camera
         * when necessary
         */

        /* follow the character around */
        CameraUpdateTarget(g_camera, e->position);
    }

    DeleteEntityDynamicArray(eda);
}

void Update(Entity *player, GameTimestep *gameTimestep)
{
    /* update logics and data here */
    /* physics */
    const GLfloat gravity = -9.81f;
    UpdateGameTimestep(gameTimestep);

    // position, velocity, acceleration
    player->acceleration.y += gravity;
    player->velocity.y += player->acceleration.y * gameTimestep->deltaTime/1000;
    player->position.y += player->velocity.y * gameTimestep->deltaTime/1000;

    /* Update movable entities */
    UpdateEntityPosition(player, gameTimestep, true);

}

void Render(GLuint vao, GLuint vbo, GLuint textureID, GLuint program,
            GLuint debugProgram, Entity *entity, Vertex *vertices)
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
    vertices;
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_OF_RECT_CORNER, g_player->data, GL_STATIC_DRAW);
    //glBufferSubData(GL_ARRAY_BUFFER, offsetof(Vertex, position), sizeof(GLfloat) * 3, vertices);

    OpenGLCheckErrors();

    GLuint modelLoc, viewLoc, projectionLoc;
#define DEBUG_SHADER 1
#if DEBUG_SHADER
    OpenGLBeginUseProgram(debugProgram, textureID);
    glEnable(GL_PROGRAM_POINT_SIZE);

    modelLoc = glGetUniformLocation(debugProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(position));
    OpenGLCheckErrors();

    viewLoc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
    OpenGLCheckErrors();

    projectionLoc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

    DrawDebugRectangle();
    OpenGLEndUseProgram();
#endif

    OpenGLBeginUseProgram(program, textureID);

    /* load uniform variable to shader program before drawing */
    modelLoc = glGetUniformLocation(program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(position));
    viewLoc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
    projectionLoc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

    OpenGLCheckErrors();
    DrawRectangle();

    /* TODO: explore a better way of doing this. reloading vertices over and
     * over might be meh. performance hit with just doing VAO might be very
     * miniscule that it's worth doing for a simpler code base?
     */

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_OF_RECT_CORNER, g_entityManager->entities[10001].data, GL_STATIC_DRAW);

    for (unsigned int i = 0; i < g_entityManager->size; i++) {
        Entity *entityToDraw;
        entityToDraw = &(g_entityManager->entities[i]);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(), entityToDraw->position)));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

        if (i == 10002) {
            //glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * NUM_OF_RECT_CORNER, g_entityManager->entities[i].data, GL_STATIC_DRAW);
            /* If the data is interleaved, you have to do multiple
             * glBufferSubData calls in order to populate all the right parts */
            glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)(offsetof(Vertex, position)), sizeof(GLfloat) * 3, g_entityManager->entities[i].data);
            glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)((Vertex*)offsetof(Vertex, position) + 1) , sizeof(GLfloat) * 3, (Vertex*)g_entityManager->entities[i].data + 1);
            glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)((Vertex*)offsetof(Vertex, position) + 2) , sizeof(GLfloat) * 3, (Vertex*)g_entityManager->entities[i].data + 2);
            glBufferSubData(GL_ARRAY_BUFFER, (GLintptr)((Vertex*)offsetof(Vertex, position) + 3) , sizeof(GLfloat) * 3, (Vertex*)g_entityManager->entities[i].data + 3);
        }
        DrawRectangle();
    }

    glBindVertexArray(0);
    OpenGLEndUseProgram();

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
            CreateRectangle(rectEntity, startingPosition, color, 1, 1);
            rectEntity->isTraversable = true;
            rectEntity->isPlayer = false;
        }
    }

    /* Let's add a non-traversable entity */
    v3 startingPosition = {-3, -3, 0};
    Entity* collisionEntity = AddNewEntity(g_entityManager, startingPosition);
    ASSERT(collisionEntity != NULL);
    CreateRectangle(collisionEntity, startingPosition, color, 2, 5);
    collisionEntity->isTraversable = false;
    collisionEntity->isPlayer = false;
}
#endif
