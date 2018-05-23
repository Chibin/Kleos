#include "main.h"
#include "game.h"

int main(int /*unused*/, char ** /*unused*/)
{
    const char *programName = "First game trial :)";
    SDL_Window *mainWindow = nullptr;
    /* our opengl context handle */
    SDL_GLContext mainContext;
    RenderAPI renderAPI = {};

    if (!WindowSetup(&mainWindow, programName) ||
        !WindowsOpenGLSetup(mainWindow, &mainContext) || !WindowsSDLTTFSetup() ||
        !LoadDLLWindows(&renderAPI))
    {
        return -1;
    }

    MainGameLoop(mainWindow, &renderAPI);
    WindowsCleanup(mainWindow, &mainContext);

    return 0;
}
