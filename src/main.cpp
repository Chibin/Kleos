#include "main.h"
#include "game.h"

#include <iostream>
#include <vector>

int main(int /*unused*/, char ** /*unused*/)
{
    const char *programName = "First game trial :)";
    SDL_Window *mainWindow = nullptr;
	SDL_Window *secondWindow = nullptr;
    /* our opengl context handle */
    SDL_GLContext mainContext;
    RenderAPI renderAPI = {};

    if (!WindowSetup(&mainWindow, programName) ||
        !WindowSetup(&secondWindow, programName) ||
        !WindowsOpenGLSetup(mainWindow, &mainContext) || !WindowsSDLTTFSetup() ||
        !LoadDLLWindows(&renderAPI))
    {
        return -1;
    }


    MainGameLoop(mainWindow, secondWindow, &renderAPI);
    WindowsCleanup(mainWindow, &mainContext);
    return 0;
}
