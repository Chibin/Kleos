#include "main.h"
#include "game.h"

#include <iostream>
#include <vector>

int main(int /*unused*/, char ** /*unused*/)
{
    const char *programName = "First game trial :)";
    SDL_Window *openglWindow = nullptr;
	SDL_Window *vulkanWindow = nullptr;
    /* our opengl context handle */
    SDL_GLContext mainContext;
    RenderAPI renderAPI = {};

    SDL_Window *firstWindow = vulkanWindow;
    SDL_Window *secondWindow = openglWindow;

    if (!WindowSetup(&openglWindow, programName) ||
        !WindowSetup(&vulkanWindow, programName) ||
        !WindowsOpenGLSetup(openglWindow, &mainContext) || !WindowsSDLTTFSetup() ||
        !LoadDLLWindows(&renderAPI))
    {
        return -1;
    }

    MainGameLoop(openglWindow, vulkanWindow, &renderAPI);
    WindowsCleanup(firstWindow, &mainContext);
    return 0;
}
