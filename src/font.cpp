#ifndef __FONT__
#define __FONT__
#include "font.h"
#include "bitmap.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push)
#pragma warning(disable : 4244)
#include "stb_image.h"
#pragma warning(pop)
#endif

TTF_Font *OpenFont()
{
    const char *file = "assets/fonts/lato/Lato-Regular.ttf";
    int ptsize = 16;

    TTF_Font *font = TTF_OpenFont(file, ptsize);
    if (font == nullptr)
    {
        TTF_SetError("Loading failed :( (code: %d)", 142);
        std::cout << "Error: " << TTF_GetError() << std::endl;

        return font;
    }

    printf("font loaded properly.\n");

    return font;
}

inline u8 ConvertColorF32ToU8(f32 color)
{
    return SafeCastToU8(color * 255);
}

inline SDL_Surface *StringToSDLSurface(TTF_Font *font, const char *msg, v4 vColor)
{
    /* TODO: Pass the color */
    SDL_Color color = { ConvertColorF32ToU8(vColor.r),
                        ConvertColorF32ToU8(vColor.g),
                        ConvertColorF32ToU8(vColor.b),
                        ConvertColorF32ToU8(vColor.a) };

    // Render the message
    SDL_Surface *surface = TTF_RenderText_Blended(font, msg, color);
    if (surface == nullptr)
    {
        printf("Failed to TTF_RenderText\n");
        return nullptr;
    }
    return surface;
}

SDL_Surface *StringToSDLSurface(TTF_Font *font, const char *msg)
{
    /* TODO: Pass the color */
    v4 color = { 1, 1, 1, 1 };
    return StringToSDLSurface(font, msg, color);
}

inline void swap(u32 *first, u32 *second)
{
    u32 temp = *first;
    *first = *second;
    *second = temp;
}

inline u32 *Get2DAddress(u32 *base, u32 xOffset, u32 yOffset, u32 maxWidth)
{
    u32 height = maxWidth * yOffset;
    return base + height + xOffset;
}

inline void OpenGLLoadBitmap(Bitmap *bitmap, GLuint textureID)
{
    OpenGLBindTexture(textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, bitmap->format, bitmap->width, bitmap->height, 0,
                 bitmap->format, GL_UNSIGNED_BYTE, bitmap->data);
}

inline void CopyData(u32 *src, u32 *dest, memory_index size)
{
    memory_index temp = size;
    while (temp--)
    {
        *dest++ = *src++;
    }
}

inline void FlipImage(u32 *pixels, u32 width, u32 height)
{

    for (uint32 yOffset = 0; yOffset < height / 2; yOffset++)
    {
        u32 heightBottomToTop = height - 1 - yOffset;
        u32 *firstPixel = pixels + yOffset * width;
        u32 *secondPixel = pixels + width * heightBottomToTop;

        for (uint32 counter = 0; counter < width; counter++)
        {
            u32 temp = *(firstPixel + counter);
            *(firstPixel + counter) = *(secondPixel + counter);
            *(secondPixel + counter) = temp;
            //u32 temp = *(firstPixel + counter);
            //SWAP_POINTER_VALUES(firstPixel, secondPixel, u32);
            //firstPixel++;
            //secondPixel++;
        }
    }
}

GLuint StringToTexture(TTF_Font *font, const char *msg)
{
    /* FIXME: Is this right? we might not be able to assume that we can just
     * pick the first glenum texture
     */
    GLuint textureID;

    SDL_Surface *surface = StringToSDLSurface(font, msg);
    assert(surface != NULL);

    GLenum format = GL_RGB;
    if (surface->format->BytesPerPixel == 4)
        format = GL_RGBA;
    u32 width = surface->w;
    u32 height = surface->h;

    FlipImage((u32 *)surface->pixels, width, height);
    textureID = OpenGLAllocateTexture(format, surface->w, surface->h,
                                      surface->pixels);

    // Clean up the surface and font
    SDL_FreeSurface(surface);
    return textureID;
}

inline void SDLSurfaceToBitmap(GameMemory *gm, SDL_Surface *surface, struct Bitmap *bitmap)
{
    u32 width = surface->w;
    u32 height = surface->h;
    GLenum format = GL_RGB;
    if (surface->format->BytesPerPixel == 4)
        format = GL_RGBA;

    FlipImage((u32 *)surface->pixels, width, height);

    bitmap->data = (u8 *)AllocateMemory(gm, width * height * sizeof(u32));
    CopyData((u32 *)surface->pixels, (u32 *)bitmap->data, width * height);
    bitmap->format = format;
    bitmap->width = width;
    bitmap->height = height;
};

void StringToBitmap(GameMemory *gm, Bitmap *bitmap, TTF_Font *font, const char *msg)
{
    /* FIXME: Is this right? we might not be able to assume that we can just
     * pick the first glenum texture
     */

    SDL_Surface *surface = StringToSDLSurface(font, msg);
    ASSERT(surface != NULL);

    SDLSurfaceToBitmap(gm, surface, bitmap);

    SDL_FreeSurface(surface);
}
#endif