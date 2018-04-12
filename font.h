#pragma once
#include <SDL2/SDL_ttf.h>

TTF_Font *OpenFont();
SDL_Surface *StringToSDLSurface(TTF_Font *font, const char *msg);
inline void CloseFont(TTF_Font *font)
{
    TTF_CloseFont(font);
}
