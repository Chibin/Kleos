#ifndef __RENDER__
#define __RENDER_

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
#pragma warning(pop)

#include <SDL.h>
#include <SDL_ttf.h>

#include <stdio.h>

/* probably need to be here? depends where we put our game logic */
#include "entity.cpp"
#include "camera.cpp"
#include "input.cpp"

#include "rectangle.cpp"

#define UPDATEANDRENDER(name) bool name(GLuint vao, GLuint textureID, GLuint program, GLuint debugProgram, Entity *entity, v2 screenResolution)
#define UPDATE(name) void name()
#define RENDER(name) void name(GLuint vao, GLuint textureID, GLuint program, GLuint debugProgram, Entity *entity)

void Render(GLuint vao, GLuint textureID, GLuint program, GLuint debugProgram, Entity *entity);
void RenderAllEntities(GLuint program);

Rect *g_testRectangle = NULL;
Camera *g_camera = NULL;
glm::mat4 *g_projection = NULL;

extern "C" UPDATEANDRENDER(UpdateAndRender)
{
    Entity *player = entity;
    SDL_Event event;
    bool continueRunning = true;

    if (!g_testRectangle) {
        v3 startingPosition = {0, 1, 0};
        g_testRectangle = CreateRectangle(startingPosition, 2, 3);
    }

    if (!g_camera) {
        g_camera = CreateCamera();
    }

    if(!g_projection) {
        float SCREEN_WIDTH = screenResolution.v[0];
        float SCREEN_HEIGHT = screenResolution.v[1];
        g_projection = (glm::mat4*)malloc(sizeof(glm::mat4));
        *g_projection = glm::infinitePerspective(45.0f, SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f);
    }

    /* NOTE: Looks very player centric right now, not sure if we need to make it
     * better later on.
     * Use keystate to get the feel for the fastest response time.
     * Keyrepeat is doesn't feel smooth enough for us to use it as a way to
     * move. The amount of keyboard repeat depends on the setting the user has
     * on their computer, so not reliable.
     */
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
    ProcessKeysHeldDown(player, keystate);

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            return false;

        if (event.type == SDL_MOUSEWHEEL)
            ProcessMouseInput(event, g_camera);

        if (event.type == SDL_KEYDOWN)
            ProcessInput(event.key.keysym.sym, &continueRunning, player);
    }

    /* TODO: One time init might be done here as the game progress ? */

    /* start with a 'clear' screen */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Render(vao, textureID, program, debugProgram, player);
    return continueRunning;
}

extern "C" UPDATE(Update)
{

}

void Render(GLuint vao, GLuint textureID, GLuint program, GLuint debugProgram, Entity *entity)
{
    /* TODO: sort materials to their own group for that specific program 
     * bind to the new program 
     */
    Entity *player = NULL;
    player = entity;

    /* Render graphics */
    glUseProgram(program);

    /* NOTE: you shouldn't call this function unless you have a shader
     * program already binded (glUseProgram)
     */
    glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // We want to repeat this pattern so we set kept it at GL_REPEAT
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);   // We want to repeat this pattern so we set kept it at GL_REPEAT
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    /* programs used first will have higher priority being shown in the
     * canvas 
     */
    glBindVertexArray(vao);

    /* This will probably be a loop per entity when drawing them... if they are
     * dynamically changing everytime???
     */
    /* TODO: remove this from here... this is just testing it out */
    glm::mat4 position = glm::mat4();
    //position = glm::translate(position, player->position);
    position = glm::translate(position, player->position);

#define DEBUG 1
#if DEBUG
    glUseProgram(debugProgram);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawElements(GL_POINTS, 6, GL_UNSIGNED_INT, 0);

    GLuint modelLoc = glGetUniformLocation(debugProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(position));
    GLuint viewLoc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
    GLuint projectionLoc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));
#endif

    RenderAllEntities(program);

    glUseProgram(program);

    /* load uniform variable to shader program before drawing */
    modelLoc = glGetUniformLocation(program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(position));
    viewLoc = glGetUniformLocation(program, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
    projectionLoc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void RenderAllEntities(GLuint program)
{

    //UseProgram(program);
    //LoadTexturesToProgram();
    //BindVertex();

    glUseProgram(program);

    /* NOTE: you shouldn't call this function unless you have a shader
     * program already binded (glUseProgram)
     */
    //glActiveTexture(GL_TEXTURE0);
    //    glBindTexture(GL_TEXTURE_2D, textureID);
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // We want to repeat this pattern so we set kept it at GL_REPEAT
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);   // We want to repeat this pattern so we set kept it at GL_REPEAT
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glUniform1i(glGetUniformLocation(program, "tex"), 0);

    ///* programs used first will have higher priority being shown in the
    // * canvas 
    // */
    //glBindVertexArray(vao);
    glm::mat4 position = glm::mat4();
    position = glm::translate(position, g_testRectangle->entity.position);

    glUseProgram(program);

    /* load uniform variable to shader program before drawing */
    GLuint modelLoc = glGetUniformLocation(program, "model");
    GLuint viewLoc = glGetUniformLocation(program, "view");
    GLuint projectionLoc = glGetUniformLocation(program, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(position));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

}

#endif
