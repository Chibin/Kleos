#ifndef __OPENGL__
#define __OPENGL__

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
