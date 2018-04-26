#pragma once

#include "main.h"
#include "GL/glew.h"
#include "game_time.h"
#include "sdl_common.h"
#include "math.h"
#include <string>
#include <iostream>

#include <windows.h>

typedef bool(__cdecl *UPDATEANDRENDER)(GLuint vao, GLuint vbo, GLuint textureID,
                                       GLuint program, GLuint debugProgram,
                                       v2 screenResolution,
                                       struct GameMetadata *gameMetadata);

struct RenderAPI
{
    std::string libraryName;
    HMODULE libHandle;
    UPDATEANDRENDER updateAndRender;
};

void _setOpenGLSettings();
void CheckSDLError(int);
bool WindowSetup(SDL_Window **mainWindow, std::string &programName);
bool WindowsOpenGLSetup(SDL_Window *mainWindow, SDL_GLContext *mainContext);
void WindowsCleanup(SDL_Window *mainWindow, SDL_GLContext *mainContext);
bool WindowsSDLTTFSetup();
bool LoadDLLWindows(RenderAPI *renderAPI);
char *GetProgramPath();
void FindFile(const char *dirPath, const char *fileRegex);

