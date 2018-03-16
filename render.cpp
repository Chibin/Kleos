#ifndef __RENDER__
#define __RENDER_

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

#define UPDATEANDRENDER(name) bool name(GLuint vao, GLuint vbo, GLuint textureID, GLuint program, GLuint debugProgram, v2 screenResolution, GLfloat *vertices, GameTimestep **gameTimestep)
#define RENDER(name) void name(GLuint vao, GLuint vbo, GLuint textureID, GLuint program, GLuint debugProgram, Entity *entity)

void Render(GLuint vao, GLuint vbo, GLuint textureID, GLuint program, GLuint debugProgram, Entity *entity, GLfloat *vertices);
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
        /* TODO: May be the entity manager should be the only one creating the entities? */
        g_entityManager = CreateEntityManger();

        Entity newPlayer;
        newPlayer.position = glm::vec3(0,0,0);

        int index = Append(g_entityManager, &newPlayer);
        printf("size %d\n", g_entityManager->size);

        /* NOTE: we don't need to free the player since we created it in the stack */
        g_player = &g_entityManager->entities[index];

        printf("size %d\n", g_entityManager->size);

    }

    if (!g_testRectangle) {
        v3 startingPosition = {-5, -5, 0};

        Entity* rectEntity = AddNewEntity(g_entityManager);
        g_testRectangle = CreateRectangle(rectEntity, startingPosition, 1, 1);
        g_rectangleVertices = CreateVertices(g_testRectangle);
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

void Update(Entity *player, GameTimestep *gameTimestep)
{
    /* update logics and data here */
    /* physics */
    const GLfloat gravity = -9.81f;
    UpdateGameTimestep(gameTimestep);

    //printf("delta: %d\n", g_gameTimestep->deltaTime);
    // position, velocity, acceleration
    player->acceleration.y += gravity;
    player->velocity.y += player->acceleration.y * gameTimestep->deltaTime/1000;
    player->position.y += player->velocity.y * gameTimestep->deltaTime/1000;

    /* can move */

    /* did entity hit floor */
    if (player->position.y < -1) {
        player->position.y = -1;
        player->velocity.y = 0;
        player->acceleration.y = 0;
    }

    player->position.x += player->velocity.x * gameTimestep->deltaTime;
    /* Apply "friction" */
    player->velocity.x = 0;


    /* TODO: bound checking for the camera such that we only move the camera
     * when necessary
     */

    /* follow the character around */
    CameraUpdateTarget(g_camera, player->position);
}

void Render(GLuint vao, GLuint vbo, GLuint textureID, GLuint program,
            GLuint debugProgram, Entity *entity, GLfloat *vertices)
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
    //position = glm::translate(position, player->position);
    position = glm::translate(position, player->position);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 20, vertices, GL_STATIC_DRAW);

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

    glDrawElements(GL_POINTS, 6, GL_UNSIGNED_INT, 0);
    OpenGLEndUseProgram();
#endif

    OpenGLBeginUseProgram(program, textureID);
    //OpenGLLoadProgramInput(program);

    /* load uniform variable to shader program before drawing */
    modelLoc = glGetUniformLocation(program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(position));
    viewLoc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
    projectionLoc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

    OpenGLCheckErrors();

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    /* TODO: explore a better way of doing this. reloading vertices over and
     * over might be meh. performance hit with just doing VAO might be very
     * miniscule that it's worth doing for a simpler code base?
     */
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    /* XXX: THIS IS BAD TO DO every frame!! :(
     * this also should be a GL_DYNAMIC_SOMETHING DRAW */
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 20, g_rectangleVertices, GL_STATIC_DRAW);

    for (unsigned int i = 0; i < g_entityManager->size; i++) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(), g_entityManager->entities[i].position)));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
    OpenGLEndUseProgram();

    OpenGLCheckErrors();
}

void RenderAllEntities(GLuint vbo)
{

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    //for(int i = 0; i < AllEntities.GetTotalEntities(); i++)
    //{
    //    GLfloat *verticies = entity->vertices;
    //    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 20, g_rectangleVertices, GL_STATIC_DRAW);
    //    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(glm::translate(glm::mat4(), glm::vec3(-10+i*5,i*2,0))));
    //    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
    //    glUniformMatrix4f(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

    //    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    //}

}

void LoadStuff()
{
    for(int i = 0; i < 100; i++) {
        for(int y = 0; y < 100; y++) {
            glm::vec3 startingPosition = glm::vec3(i, y, 0);
            Entity* rectEntity = AddNewEntity(g_entityManager);
            ASSERT(rectEntity != NULL);
            rectEntity->position = startingPosition;
        }
    }
}

#endif
