#pragma once

#include "sdl_common.h"

TTF_Font *OpenFont();
SDL_Surface *StringToSDLSurface(TTF_Font *font, const char *msg);
inline void CloseFont(TTF_Font *font)
{
    TTF_CloseFont(font);
}
