#include "game.h"
#include "game_memory.h"
#include "game_metadata.h"
#include "logger.h"
#include "main.h"
#include <GL/glew.h>

#define ProcessOpenGLErrors() _processOpenGLErrors(__FILE__, __LINE__)

void _processOpenGLErrors(const char *file, int line);

void MainGameLoop(SDL_Window *mainWindow, RenderAPI *renderAPI)
{
    /* sanity check */
    ASSERT(sizeof(real32) == sizeof(GLfloat));

    /* TODO: this is probably not the right spot for this */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    bool continueRunning = true;

    struct GameMetadata gameMetadata = {};
    gameMetadata.font = nullptr;
    gameMetadata.maxBlockSize = GIGABYTE(1);
    gameMetadata.base = (u8 *)malloc(gameMetadata.maxBlockSize);

    u32 tempSize = MEGABYTE(8);
    gameMetadata.temporaryMemory.base = (u8 *)AllocateMemory(&gameMetadata, tempSize);
    gameMetadata.temporaryMemory.maxSize = tempSize;

    u32 reservedSize = MEGABYTE(900);
    gameMetadata.reservedMemory.base = (u8 *)AllocateMemory(&gameMetadata, reservedSize);
    gameMetadata.reservedMemory.maxSize = reservedSize;

    gameMetadata.vaoID = 0;
    gameMetadata.eboID = 0;
    gameMetadata.vboID = 0;
    gameMetadata.screenResolution = v2{ SCREEN_WIDTH, SCREEN_HEIGHT };
    gameMetadata.initFromGameUpdateAndRender = false;

    gameMetadata.sentinelNode = {};
    gameMetadata.sentinelNode.next = &gameMetadata.sentinelNode;
    gameMetadata.sentinelNode.prev = &gameMetadata.sentinelNode;
    gameMetadata.program = 0;
    gameMetadata.debugProgram = 0;

    FindFile(GetProgramPath(), "render*dll");

    while (continueRunning)
    {
        continueRunning = ((renderAPI->updateAndRender)(&gameMetadata) != 0);
        ProcessOpenGLErrors();

        /* equivalent to glswapbuffer? */
        SDL_GL_SwapWindow(mainWindow);
    }
}

void _processOpenGLErrors(const char *file, int line)
{
    /* TODO: change how we print the error? */
    // Process/log the error.
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::string error;

        printf("error detected at %s:%d:\n", file, line);

        switch (err)
        {
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            printf("something bad happened. "
                   "unknown error: %d\n",
                   err);
            break;
        }

        printf("something bad happened: %s\n", error.c_str());
    }
}
