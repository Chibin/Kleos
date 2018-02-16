#ifndef __TEXTURE__
#define __TEXTURE__

inline void ImageToTexture(GLuint textureID, const char * ImageName);

inline
void ImageToTexture(GLuint textureID, const char * ImageName)
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

    glBindTexture(GL_TEXTURE_2D, textureID);

    int textureFormat = GL_RGB;
    if(componentsPerPixel== 4)
        textureFormat = GL_RGBA;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                 textureFormat, GL_UNSIGNED_BYTE, image);

    stbi_image_free(image);

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

inline
void StringToTexture(GLuint textureID, TTF_Font *font, const char *msg)
{
    /* FIXME: Is this right? we might not be able to assume that we can just
     * pick the first glenum texture
     */
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    SDL_Surface *surface = StringToSDLSurface(font, msg);
    assert(surface != NULL);

    int textureFormat = GL_RGB;
    if(surface->format->BytesPerPixel == 4)
        textureFormat = GL_RGBA;

    /* https://www.khronos.org/opengl/wiki/Common_Mistakes
     * Creating a complete texture
     */
    glTexImage2D(GL_TEXTURE_2D, 0, textureFormat, surface->w, surface->h, 0, 
                 textureFormat, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_2D,  GL_GENERATE_MIPMAP, GL_TRUE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    //Clean up the surfaceace and font
    SDL_FreeSurface(surface);
}
#endif
