#ifndef __OPENGL__
#define __OPENGL__

#include "math.h"
#include <GL/glew.h>

#define OpenGLCheckErrors() _defined_openGLCheckErrors(__FILE__, __LINE__)

struct Vertex
{
    /* 3 + 4 + 3 + 2  = 12 */
    GLfloat position[3];
    GLfloat color[4];
    GLfloat normal[3];
    GLfloat uv[2];
};

struct Program
{
    GLuint handle;

    /* common things to get passed into a shader program
     * position
     * camera view
     * projection
     */
};

void OpenGLBeginUseProgram(GLuint program, GLuint textureID = 0);
void OpenGLEndUseProgram();
void OpenGLCreateVAO(GLuint &vao, GLuint &vbo, uint32 vboSize,
                     Vertex *vboVertices, GLuint &ebo, uint32 eboSize,
                     GLuint *eboVertices, GLenum vboUsage = GL_DYNAMIC_DRAW,
                     GLenum eboUsage = GL_STATIC_DRAW);
GLuint *OpenGLAllocateTexture(int textureFormat, int width, int height,
                              void *data);
void _defined_openGLCheckErrors(const char *file, int line);

#endif
