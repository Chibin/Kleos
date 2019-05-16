#ifndef __FONT__
#define __FONT__
#include "font.h"

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
    int ptsize = 144;

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
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font, msg, color);
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

inline void CopyData(u32 *src, u32 *dest, memory_index size)
{
    memory_index temp = size;
    while (temp--) // NOLINT
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

#if 1
            *(firstPixel + counter) ^= *(secondPixel + counter)
                ^= *(firstPixel + counter) ^= *(secondPixel + counter);
#else
            u32 temp = *(firstPixel + counter);
            *(firstPixel + counter) = *(secondPixel + counter);
            *(secondPixel + counter) = temp;
            u32 temp = *(firstPixel + counter);
            SWAP_POINTER_VALUES(firstPixel, secondPixel, u32);
            firstPixel++;
            secondPixel++;
#endif
        }
    }
}

inline void SDLSurfaceToBitmap(SDL_Surface *surface, struct Bitmap *bitmap)
{
    u32 width = surface->w;
    u32 height = surface->h;
    u32 pitch = surface->pitch;
    GLenum format = GL_RGB;
    if (surface->format->BytesPerPixel == 4)
    {
        format = GL_RGBA;
    }

    SDL_LockSurface(surface);

#if 1
    /* Need to use this if it's OpenGL only */
    FlipImage((u32 *)surface->pixels, width, height);
#endif

    bitmap->data = (u8 *)surface->pixels;
    bitmap->size = width * height * 4;
    bitmap->format = format;
    bitmap->width = width;
    bitmap->height = height;
    bitmap->pitch = pitch;
};

void StringToTexture(TTF_Font *font, const char *msg)
{
    /* FIXME: Is this right? we might not be able to assume that we can just
     * pick the first glenum texture
     */
    SDL_Surface *surface = StringToSDLSurface(font, msg);
    assert(surface != nullptr);

    GLenum format = GL_RGB;
    if (surface->format->BytesPerPixel == 4)
    {
        format = GL_RGBA;
    }

    ASSERT(surface->format->BytesPerPixel == 4);

    u32 width = surface->w;
    u32 height = surface->h;

    FlipImage((u32 *)surface->pixels, width, height);

    // Clean up the surface and font
    SDL_FreeSurface(surface);
}

void StringToBitmap(Bitmap *bitmap, TTF_Font *font, const char *msg)
{
    /* FIXME: Is this right? we might not be able to assume that we can just
     * pick the first glenum texture
     */

    SDL_Surface *surface = StringToSDLSurface(font, msg);
    ASSERT(surface != nullptr);

    SDLSurfaceToBitmap(surface, bitmap);

    //SDL_FreeSurface(surface);
}

FontBitmapInfo GetFontPixelInfo(s32 character)
{
    const f32 fontMaxPixelHeight = 174;
    FontBitmapInfo result = {};

    switch(character)
    {
        case '0':
            result.pixelStart = v2{0, fontMaxPixelHeight};
            result.pixelEnd = v2{83, fontMaxPixelHeight};
            break;
        case '1':
            result.pixelStart = v2{84, fontMaxPixelHeight};
            result.pixelEnd = v2{166, fontMaxPixelHeight};
            break;
        case '2':
            result.pixelStart = v2{167, fontMaxPixelHeight};
            result.pixelEnd = v2{249, fontMaxPixelHeight};
            break;
        case '3':
            result.pixelStart = v2{250, fontMaxPixelHeight};
            result.pixelEnd = v2{332, fontMaxPixelHeight};
            break;
        case '4':
            result.pixelStart = v2{333, fontMaxPixelHeight};
            result.pixelEnd = v2{415, fontMaxPixelHeight};
            break;
        case '5':
            result.pixelStart = v2{416, fontMaxPixelHeight};
            result.pixelEnd = v2{498, fontMaxPixelHeight};
            break;
        case '6':
            result.pixelStart = v2{499, fontMaxPixelHeight};
            result.pixelEnd = v2{581, fontMaxPixelHeight};
            break;
        case '7':
            result.pixelStart = v2{588, fontMaxPixelHeight};
            result.pixelEnd = v2{671, fontMaxPixelHeight};
            break;
        case '8':
            result.pixelStart = v2{671, fontMaxPixelHeight};
            result.pixelEnd = v2{753, fontMaxPixelHeight};
            break;
        case '9':
            result.pixelStart = v2{754, fontMaxPixelHeight};
            result.pixelEnd = v2{836, fontMaxPixelHeight};
            break;
        case 'A':
            result.pixelStart = v2{831, fontMaxPixelHeight};
            result.pixelEnd = v2{945, fontMaxPixelHeight};
            break;
        case 'B':
            result.pixelStart = v2{946, fontMaxPixelHeight};
            result.pixelEnd = v2{1025, fontMaxPixelHeight};
            break;
        case 'C':
            result.pixelStart = v2{1026, fontMaxPixelHeight};
            result.pixelEnd = v2{1130, fontMaxPixelHeight};
            break;
        case 'D':
            result.pixelStart = v2{1140, fontMaxPixelHeight};
            result.pixelEnd = v2{1230, fontMaxPixelHeight};
            break;
        case 'E':
            result.pixelStart = v2{1247, fontMaxPixelHeight};
            result.pixelEnd = v2{1325, fontMaxPixelHeight};
            break;
        case 'F':
            result.pixelStart = v2{1320, fontMaxPixelHeight};
            result.pixelEnd = v2{1405, fontMaxPixelHeight};
            break;
        case 'G':
            result.pixelStart = v2{1400, fontMaxPixelHeight};
            result.pixelEnd = v2{1505, fontMaxPixelHeight};
            break;
        case 'H':
            result.pixelStart = v2{1506, fontMaxPixelHeight};
            result.pixelEnd = v2{1615, fontMaxPixelHeight};
            break;
        case 'I':
            result.pixelStart = v2{1616, fontMaxPixelHeight};
            result.pixelEnd = v2{1655, fontMaxPixelHeight};
            break;
        case 'J':
            result.pixelStart = v2{1650, fontMaxPixelHeight};
            result.pixelEnd = v2{1720, fontMaxPixelHeight};
            break;
        case 'K':
            result.pixelStart = v2{1721, fontMaxPixelHeight};
            result.pixelEnd = v2{1815, fontMaxPixelHeight};
            break;
        case 'L':
            result.pixelStart = v2{1816, fontMaxPixelHeight};
            result.pixelEnd = v2{1895, fontMaxPixelHeight};
            break;
        case 'M':
            result.pixelStart = v2{1887, fontMaxPixelHeight};
            result.pixelEnd = v2{2015, fontMaxPixelHeight};
            break;
        case 'N':
            result.pixelStart = v2{2020, fontMaxPixelHeight};
            result.pixelEnd = v2{2130, fontMaxPixelHeight};
            break;
        case 'O':
            result.pixelStart = v2{2136, fontMaxPixelHeight};
            result.pixelEnd = v2{2245, fontMaxPixelHeight};
            break;
        case 'P':
            result.pixelStart = v2{2246, fontMaxPixelHeight};
            result.pixelEnd = v2{2333, fontMaxPixelHeight};
            break;
        case 'Q':
            result.pixelStart = v2{2325, fontMaxPixelHeight};
            result.pixelEnd = v2{2450, fontMaxPixelHeight};
            break;
        case 'R':
            result.pixelStart = v2{2445, fontMaxPixelHeight};
            result.pixelEnd = v2{2535, fontMaxPixelHeight};
            break;
        case 'S':
            result.pixelStart = v2{2535, fontMaxPixelHeight};
            result.pixelEnd = v2{2615, fontMaxPixelHeight};
            break;
        case 'T':
            result.pixelStart = v2{2610, fontMaxPixelHeight};
            result.pixelEnd = v2{2701, fontMaxPixelHeight};
            break;
        case 'U':
            result.pixelStart = v2{2701, fontMaxPixelHeight};
            result.pixelEnd = v2{2801, fontMaxPixelHeight};
            break;
        case 'V':
            result.pixelStart = v2{2798, fontMaxPixelHeight};
            result.pixelEnd = v2{2904, fontMaxPixelHeight};
            break;
        case 'W':
            result.pixelStart = v2{2904, fontMaxPixelHeight};
            result.pixelEnd = v2{3052, fontMaxPixelHeight};
            break;
        case 'X':
            result.pixelStart = v2{3052, fontMaxPixelHeight};
            result.pixelEnd = v2{3142, fontMaxPixelHeight};
            break;
        case 'Y':
            result.pixelStart = v2{3142, fontMaxPixelHeight};
            result.pixelEnd = v2{3237, fontMaxPixelHeight};
            break;
        case 'Z':
            result.pixelStart = v2{3233, fontMaxPixelHeight};
            result.pixelEnd = v2{3321, fontMaxPixelHeight};
            break;
        case 'a':
            result.pixelStart = v2{3321, fontMaxPixelHeight};
            result.pixelEnd = v2{3401, fontMaxPixelHeight};
            break;
        case 'b':
            result.pixelStart = v2{3401, fontMaxPixelHeight};
            result.pixelEnd = v2{3476, fontMaxPixelHeight};
            break;
        case 'c':
            result.pixelStart = v2{3481, fontMaxPixelHeight};
            result.pixelEnd = v2{3538, fontMaxPixelHeight};
            break;
        case 'd':
            result.pixelStart = v2{3548, fontMaxPixelHeight};
            result.pixelEnd = v2{3615, fontMaxPixelHeight};
            break;
        case 'e':
            result.pixelStart = v2{3629, fontMaxPixelHeight};
            result.pixelEnd = v2{3700, fontMaxPixelHeight};
            break;
        case 'f':
            result.pixelStart = v2{3703, fontMaxPixelHeight};
            result.pixelEnd = v2{3753, fontMaxPixelHeight};
            break;
        case 'i':
            result.pixelStart = v2{3902, fontMaxPixelHeight};
            result.pixelEnd = v2{3933, fontMaxPixelHeight};
            break;
        case 'l':
            result.pixelStart = v2{4055, fontMaxPixelHeight};
            result.pixelEnd = v2{4083, fontMaxPixelHeight};
            break;
        case 'm':
            result.pixelStart = v2{4083, fontMaxPixelHeight};
            result.pixelEnd = v2{4203, fontMaxPixelHeight};
            break;
        case 'r':
            result.pixelStart = v2{4503, fontMaxPixelHeight};
            result.pixelEnd = v2{4503, fontMaxPixelHeight};
        case 's':
            result.pixelStart = v2{4583, fontMaxPixelHeight};
            result.pixelEnd = v2{4632, fontMaxPixelHeight};
            break;
        case 't':
            result.pixelStart = v2{4633, fontMaxPixelHeight};
            result.pixelEnd = v2{4693, fontMaxPixelHeight};
            break;
        case 'y':
            result.pixelStart = v2{5033, fontMaxPixelHeight};
            result.pixelEnd = v2{5103, fontMaxPixelHeight};
            break;
        case '/':
            result.pixelStart = v2{5163, fontMaxPixelHeight};
            result.pixelEnd = v2{5240, fontMaxPixelHeight};
            break;
        case '.':
            result.pixelStart = v2{5240, fontMaxPixelHeight};
            result.pixelEnd = v2{5260, fontMaxPixelHeight};
            break;
        case ',':
            result.pixelStart = v2{5300, fontMaxPixelHeight};
            result.pixelEnd = v2{5300, fontMaxPixelHeight};
            break;
        case '!':
            result.pixelStart = v2{5300, fontMaxPixelHeight};
            result.pixelEnd = v2{5345, fontMaxPixelHeight};
            break;
        case '-':
            result.pixelStart = v2{5350, fontMaxPixelHeight};
            result.pixelEnd = v2{5390, fontMaxPixelHeight};
            break;
        default:
            break;
    }

    return result;
}
#endif
