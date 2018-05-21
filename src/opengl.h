#ifndef __OPENGL__
#define __OPENGL__

#include "math.h"
#include <GL/glew.h>

#define OpenGLCheckErrors() _defined_openGLCheckErrors(__FILE__, __LINE__)

/* TODO: may be a macro that we can add on all opengl functions to detect opengl errors in debug mode */

#pragma warning(push)
#pragma warning(disable : 4201)
struct Vertex
{
    /* 3 + 4 + 3 + 2  = 12 */
    union {
        struct
        {
            union v3 vPosition;
        };
        GLfloat position[3];
    };

    union {
        struct
        {
            union v4 vColor;
        };
        GLfloat color[4];
    };

    union {
        struct
        {
            union v3 vNormal;
        };
        GLfloat normal[3];
    };

    union {
        struct
        {
            union v2 vUv;
        };
        GLfloat uv[2];
    };
};
#pragma warning(pop)

struct TextureParam
{
    s32 magFilter;
    s32 minFilter;
};

inline b32 operator==(TextureParam a, TextureParam b)
{
    return (a.magFilter == b.magFilter) && (a.minFilter == b.minFilter);
}

inline b32 operator!=(TextureParam a, TextureParam b)
{
    return (a.magFilter != b.magFilter) || (a.minFilter != b.minFilter);
}

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
void OpenGLCreateVAO(GLuint &vao, GLuint &vbo, uint32 vboSize, // NOLINT
                     Vertex *vboVertices, GLuint &ebo, uint32 eboSize, // NOLINT
                     GLuint *eboVertices, GLenum vboUsage = GL_DYNAMIC_DRAW,
                     GLenum eboUsage = GL_STATIC_DRAW);
GLuint OpenGLAllocateTexture(int textureFormat, int width, int height,
                             void *data);
void _defined_openGLCheckErrors(const char *file, int line);

void OpenGLBindTexture(GLuint textureID);
#endif
