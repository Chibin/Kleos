#ifndef __TEXTURE__
#define __TEXTURE__

inline void
imageToTexture(GLuint TextureId, const char * ImageName)
{

    int width, height, componentsPerPixel;

    /* PNGs are normally 'flipped'.
     * This happens because OpenGL expects the 0.0 coordinate on the y-axis to
     * be on the bottom side of the image, but images usually have 0.0 at the
     * top of the y-axis
     */
    int ImageLen = strlen(ImageName);
    if (ImageLen >= 5 && strcmp(ImageName + ImageLen - 5, ".png"))
        stbi_set_flip_vertically_on_load(true);
        
    unsigned char *image = stbi_load(ImageName, &width, 
                                     &height, &componentsPerPixel, 0);

    glBindTexture(GL_TEXTURE_2D, TextureId);
    if (componentsPerPixel == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, image);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, image);
    }

    stbi_image_free(image);

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}


#endif
