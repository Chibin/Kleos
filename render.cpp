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
#pragma warning(pop)

#include <SDL.h>
#include <SDL_ttf.h>

#include <stdio.h>

/* probably need to be here? depends where we put our game logic */
#include "entity.cpp"
#include "input.cpp"

#define UPDATEANDRENDER(name) bool name(GLuint vao, GLuint textureID, GLuint program, GLuint debugProgram, entity *Entity)
#define UPDATE(name) void name()
#define RENDER(name) void name(GLuint vao, GLuint textureID, GLuint program, GLuint debugProgram, entity *Entity)

void Render(GLuint vao, GLuint textureID, GLuint program, GLuint debugProgram, entity *Entity);

extern "C" UPDATEANDRENDER(UpdateAndRender)
{
    entity *player = Entity;
    SDL_Event event;
    bool continueRunning = true;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            return false;

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

void Render(GLuint vao, GLuint textureID, GLuint program, GLuint debugProgram, entity *Entity)
{
    /* TODO: sort materials to their own group for that specific program 
     * bind to the new program 
     */
    entity *player = NULL;
    player = Entity;

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

    glUseProgram(debugProgram);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawElements(GL_POINTS, 6, GL_UNSIGNED_INT, 0);

    /* TODO: remove this from here... this is just testing it out */
    glm::mat4 position = glm::mat4();
    position = glm::translate(position, player->position);
    GLuint modelLoc = glGetUniformLocation(debugProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(position));

    glUseProgram(program);

    /* load uniform variable to shader program before drawing */
    modelLoc = glGetUniformLocation(program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(position));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
#endif
