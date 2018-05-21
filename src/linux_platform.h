#pragma once

#include "GL/glew.h"
#include "game_time.h"
#include "math.h"
#include <SDL2/SDL.h>
#include <string>

typedef void *(*UPDATEANDRENDER)(struct GameMetadata *gameMetadata);

struct RenderAPI
{
    void *libHandle;
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
void PrintFileTimeStamp(char searchData);
void FindFile(const char *dirPath, const char *fileRegex);
void GetLatestFile();
void ListFiles(const char *dirPath);
