#pragma once

#include "bitmap.h"
#include "game_memory.h"
#include "math.h"
#include "sdl_common.h"
#include <iostream>

#include "opengl.h"
#include <cstring>

#define SWAP_POINTER_VALUES(x, y, T) \
    do                               \
    {                                \
        T SWAP = *(x);               \
        *(x) = *(y);                 \
        *(y) = SWAP;                 \
    } while (0)

TTF_Font *OpenFont();
SDL_Surface *StringToSDLSurface(TTF_Font *font, const char *msg);
inline void CopyData(u32 *src, u32 *dest, memory_index size);
inline void FlipImage(u32 *pixels, u32 width, u32 height);
inline void SDLSurfaceToBitmap(SDL_Surface *surface, struct Bitmap *bitmap);
inline SDL_Surface *StringToSDLSurface(TTF_Font *font, const char *msg, v4 vColor);
inline u8 ConvertColorF32ToU8(f32 color);
void StringToBitmap(Bitmap *bitmap, TTF_Font *font, const char *msg);
GLuint StringToTexture(TTF_Font *font, const char *msg);

inline void CloseFont(TTF_Font *font)
{
    TTF_CloseFont(font);
}
struct Texture
{
    unsigned int width;
    unsigned int height;
    GLenum format;
};
