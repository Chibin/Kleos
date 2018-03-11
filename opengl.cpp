#ifndef __OPENGL__
#define __OPENGL__

#define OpenGLCheckErrors() _defined_openGLCheckErrors(__FILE__, __LINE__)

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

void _defined_openGLCheckErrors(const char *file, int line)
{
    /* TODO: change how we print the error? */
    // Process/log the error.
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR){
        std::string error;

        printf("error detected at %s:%d:\n", file, line);

        switch(err) {
            case GL_INVALID_OPERATION: error="INVALID_OPERATION"; break;
            case GL_INVALID_ENUM:      error="INVALID_ENUM";      break;
            case GL_INVALID_VALUE:     error="INVALID_VALUE";     break;
            case GL_OUT_OF_MEMORY:     error="OUT_OF_MEMORY";     break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                                       error="INVALID_FRAMEBUFFER_OPERATION";  break;
            default: printf("something bad happened. "
                            "unknown error: %d\n", err); break;
        }

        printf("an error occured: %s\n", error.c_str());
        ASSERT(strlen(error.c_str()) == 0);
    }
}
#endif
