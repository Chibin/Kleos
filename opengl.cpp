#include "opengl.h"
#include "main.h"
#include "sdl_common.h"
#include <cstdio>
#include <cstdlib>
#include <string>

inline void OpenGLBindTexture(GLuint textureID)
{
    ASSERT(textureID != 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    /* TODO:Pass in tex? */
    //glUniform1i(glGetUniformLocation(program, "tex"), 0);
}

/* texture object 0 is a special number. If a texture object gets
 * deleted, then the texture object gets rebounded to 0.
 */
void OpenGLBeginUseProgram(GLuint program, GLuint textureID)
{
    glUseProgram(program);

    /* NOTE: you shouldn't call this function unless you have a shader
     * program already binded (glUseProgram)
     */
    OpenGLBindTexture(textureID);
}

void OpenGLEndUseProgram()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLCreateVAO(GLuint &vao, GLuint &vbo, uint32 vboSize,
                     Vertex *vboVertices, GLuint &ebo, uint32 eboSize,
                     GLuint *eboVertices, GLenum vboUsage, GLenum eboUsage)
{
    /*  Initialization code (done once (unless your object frequently changes))
     */
    // 1. Bind Vertex Array Object
    glBindVertexArray(vao);
    // 2. Copy our vertices array in a buffer for OpenGL to use

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vboSize, vboVertices, vboUsage);
    /* This determines whether you'll use glDrawElements or glDrawArrays.
     * NOTE: assuming to always use ebo
     */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, eboSize, eboVertices, eboUsage);

    /* common attributes */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid *)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid *)offsetof(Vertex, color));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (GLvoid *)offsetof(Vertex, uv));

    /* used for GLSL 4.x */
#if 0
        glEnableVertexAttribArray(0);
        glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
        glVertexAttribBinding(0, 0);
        glEnableVertexAttribArray(1);
        glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
        glVertexAttribBinding(1, 0);
        glEnableVertexAttribArray(2);
        glVertexAttribFormat(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(Vertex, color));
        glVertexAttribBinding(2, 0);
#endif

    /* Unbind the VAO (NOT THE EBO); We need to make sure that we always unbind.
     * Otherwise, we might accidentally save some unwanted commands into
     * the vertext array object.
     */

    /*  It is common practice to unbind OpenGL objects when we're done
     *  configuring them so we don't mistakenly (mis)configure them
     *  elsewhere. */
    glBindVertexArray(0);
}

GLuint OpenGLAllocateTexture(int textureFormat, int width, int height,
                              void *data)
{
    /* returns a texture ID "handle" to
     * access it later in OpenGL.
     */

    GLuint textureID;
    glGenTextures(1, &textureID);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, width, height, 0,
                 textureFormat, GL_UNSIGNED_BYTE, data);

    switch (textureFormat)
    {
    case GL_RGBA:
        printf("RGBA\n");
        /* TODO: Will need to remove later on */
        OpenGLCheckErrors();
        break;

    case GL_RGB:
        printf("RGB\n");
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        /* TODO: Will need to remove later on */
        OpenGLCheckErrors();
        break;

    default:
        printf("Unknown\n");
        break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

inline void OpenGLUpdateTextureParameter(TextureParam *textureParam)
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureParam->minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureParam->magFilter);
}

void _defined_openGLCheckErrors(const char *file, int line)
{
    /* TODO: change how we print the error? */
    // Process/log the error.
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::string error;

        printf("error detected at %s:%d:\n", file, line);

        switch (err)
        {
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            printf("something bad happened. "
                   "unknown error: %d\n",
                   err);
            break;
        }

        printf("an error occured: %s\n", error.c_str());
        ASSERT(strlen(error.c_str()) == 0);
    }
}
