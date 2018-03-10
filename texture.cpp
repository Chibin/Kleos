#ifndef __TEXTURE__
#define __TEXTURE__

struct Texture {
    unsigned int width;
    unsigned int height;
    GLenum format;
};


inline
GLuint * ImageToTexture(const char * ImageName)
{

    GLuint *textureID = NULL;

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

    int textureFormat = GL_RGB;
    if(componentsPerPixel== 4)
        textureFormat = GL_RGBA;

    textureID =  OpenGLAllocateTexture(textureFormat, width, height, image);

    stbi_image_free(image);

    return textureID;
}

inline
GLuint * StringToTexture(TTF_Font *font, const char *msg)
{
    /* FIXME: Is this right? we might not be able to assume that we can just
     * pick the first glenum texture
     */
    GLuint *textureID = NULL;

    SDL_Surface *surface = StringToSDLSurface(font, msg);
    assert(surface != NULL);

    Texture *texture = (Texture*)malloc(sizeof(Texture));
    ASSERT(texture != NULL);

    texture->format = GL_RGB;
    if(surface->format->BytesPerPixel == 4)
        texture->format = GL_RGBA;
    texture->width = surface->w;
    texture->height = surface->h;

    textureID = OpenGLAllocateTexture(texture->format, surface->w, surface->h,
                                      surface->pixels);

    //Clean up the surfaceace and font
    SDL_FreeSurface(surface);
    return textureID;
}
#endif
