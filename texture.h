#pragma once

#include "math.h"
#include "font.h"
#include "main.h"
#include "opengl.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <assert.h>
#include <cstring>

#include "sdl_common.h"

struct Texture
{
    unsigned int width;
    unsigned int height;
    GLenum format;
};

inline GLuint *ImageToTexture(const char *ImageName)
{

    GLuint *textureID = NULL;

    int width, height, componentsPerPixel;

    /* PNGs are normally 'flipped'.
     * This happens because OpenGL expects the 0.0 coordinate on the y-axis to
     * be on the bottom side of the image, but images usually have 0.0 at the
     * top of the y-axis
     */
    size_t ImageLen = strlen(ImageName);
    if (ImageLen >= 5 && strcmp(ImageName + ImageLen - 5, ".png"))
        stbi_set_flip_vertically_on_load(true);

    unsigned char *image =
        stbi_load(ImageName, &width, &height, &componentsPerPixel, 0);

    int textureFormat = GL_RGB;
    if (componentsPerPixel == 4)
        textureFormat = GL_RGBA;

    textureID = OpenGLAllocateTexture(textureFormat, width, height, image);
    OpenGLCheckErrors();

    stbi_image_free(image);

    return textureID;
}

inline void swap(u32 *first, u32 *second)
{
    u32 temp = *first;
    *first = *second;
    *second = temp;
}

#define SWAP_POINTER_VALUES(x, y, T) do { T SWAP = *x; *x = *y; *y = SWAP; } while (0)

inline u32 *Get2DAddress(u32 *base, u32 xOffset, u32 yOffset, u32 maxWidth)
{
    u32 height = maxWidth * yOffset;
    return base + height + xOffset;
}

inline void FlipImage(u32 *pixels, u32 width, u32 height)
{

    for(uint32 yOffset = 0; yOffset < height/2; yOffset++) {
        u32 heightBottomToTop = height - 1 - yOffset;
        u32 *firstPixel = pixels + yOffset * width;
        u32 *secondPixel = pixels + width * heightBottomToTop;
        for(uint32 counter = 0; counter < width; counter++) {
            SWAP_POINTER_VALUES(firstPixel, secondPixel, u32);
            firstPixel++;
            secondPixel++;
        }
    }

}

inline GLuint *StringToTexture(TTF_Font *font, const char *msg)
{
    /* FIXME: Is this right? we might not be able to assume that we can just
     * pick the first glenum texture
     */
    GLuint *textureID = NULL;

    SDL_Surface *surface = StringToSDLSurface(font, msg);
    assert(surface != NULL);

    Texture texture = {};
    texture.format = GL_RGB;
    if (surface->format->BytesPerPixel == 4)
        texture.format = GL_RGBA;
    texture.width = surface->w;
    texture.height = surface->h;

    FlipImage((u32 *)surface->pixels, texture.width, texture.height);
    textureID = OpenGLAllocateTexture(texture.format, surface->w, surface->h,
                                      surface->pixels);

    // Clean up the surfaceace and font
    SDL_FreeSurface(surface);
    return textureID;
}
