#ifndef __OPENGL__
#define __OPENGL__

struct Program {
    GLuint handle;

    /* common things to get passed into a shader program
     * position
     * camera view
     * projection
     */
};

/* texture object 0 is a special number. If a texture object gets
 * deleted, then the texture object gets rebounded to 0.
 */
void OpenGLBeginUseProgram(GLuint program, GLuint textureID = 0)
{
    glUseProgram(program);

    /* NOTE: you shouldn't call this function unless you have a shader
     * program already binded (glUseProgram)
     */
    if (textureID != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        /* TODO:Pass in tex? */
        glUniform1i(glGetUniformLocation(program, "tex"), 0);
    }
}

void OpenGLEndUseProgram()
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint * OpenGLAllocateTexture(int textureFormat, int width, int height, void * data)
{
    /* returns a texture ID "handle" to
     * access it later in OpenGL.
     */

    GLuint *textureID = (GLuint*)malloc(sizeof(GLuint));
    glGenTextures(1, textureID);

    ASSERT(textureID != NULL);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *textureID);

    glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, width, height, 0,
                 textureFormat, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D,  GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}
#endif
