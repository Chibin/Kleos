#pragma once

#include "GL/glew.h"
#include "game_time.h"
#include "main.h"
#include "math.h"
#include "sdl_common.h"
#include <iostream>
#include <string>

#include <windows.h>

typedef bool(__cdecl *UPDATEANDRENDER)(struct GameMetadata *gameMetadata);

struct RenderAPI
{
    HMODULE libHandle;
    UPDATEANDRENDER updateAndRender;
};

void _setOpenGLSettings();
void CheckSDLError(int /*line*/);
bool WindowSetup(SDL_Window **mainWindow, const std::string &programName);
bool WindowsOpenGLSetup(SDL_Window *mainWindow, SDL_GLContext *mainContext);
void WindowsCleanup(SDL_Window *mainWindow, SDL_GLContext *mainContext);
bool WindowsSDLTTFSetup();
bool LoadDLLWindows(RenderAPI *renderAPI);
char *GetProgramPath();
void FindFile(const char *dirPath, const char *fileRegex);
