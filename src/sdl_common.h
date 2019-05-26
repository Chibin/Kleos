#pragma once

#ifdef WIN32
#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_ttf.h>
#endif

#define ASSERT(condition) SDL_assert(condition) // NOLINT
