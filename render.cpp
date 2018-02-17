#ifndef __RENDER__
#define __RENDER_

#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>


#define RENDER(name) void name(GLuint vao, GLuint textureID, GLuint program, GLuint debugProgram)

extern "C" RENDER(Render)
{
    /* Render graphics */
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // We want to repeat this pattern so we set kept it at GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);   // We want to repeat this pattern so we set kept it at GL_REPEAT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    /* NOTE: you shouldn't call this function unless you already have a shader
     * program already binded (glUseProgram)
     */
    glUniform1i(glGetUniformLocation(program, "tex"), 0);

    /* programs used first will have higher priority being shown in the
     * canvas 
     */
    glBindVertexArray(vao);
    glUseProgram(debugProgram);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawElements(GL_POINTS, 6, GL_UNSIGNED_INT, 0);

    glUseProgram(program);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

#endif
