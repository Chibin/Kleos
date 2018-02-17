
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <stdio.h>
#include <assert.h>
#include <windows.h>

#include <SDL.h>
#include <SDL_ttf.h>

#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "logger.cpp"
#include "font.cpp"
#include "texture.cpp"
#include "shaders.cpp"
#include "game.cpp"

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

std::string programName = "First game trial :)";
SDL_Window *mainWindow;

// Our opengl context handle
SDL_GLContext mainContext;

void SetOpenGLSettings();
void Cleanup();
void CheckSDLError(int);
bool WindowSetup();
bool OpenGLSetup();
bool SDLTTFSetup();

int main(int argc, char *argv[])
{
    if(!WindowSetup() || !OpenGLSetup() || !SDLTTFSetup())
        return -1;

    MainGameLoop(mainWindow);
    Cleanup();

    return 0;
}

bool WindowSetup(){
    // Initialize SDL's video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "Failed to init SDL\n";
        return false;
    }

    mainWindow = SDL_CreateWindow(programName.c_str(), SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);

    if (!mainWindow)
    {
        std::cout << "Unable to create window\n";
        CheckSDLError(__LINE__);
        return false;
    }

    // Create our opengl context and attach it to our window
    mainContext = SDL_GL_CreateContext(mainWindow);
    return true;
}

bool SDLTTFSetup()
{
    if (TTF_Init() < 0) {
        printf("TTF failed to init\n");
        return false;
    }

    printf("TTF init complete\n");

    return true;
}

bool OpenGLSetup()
{
    SetOpenGLSettings();

    /* initialize to start using opengl */
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if( glewError != GLEW_OK ) {
        std::cout << "Error initializing GLEW! " << glewGetErrorString( glewError ) << std::endl;
        return false;
    }

    if( SDL_GL_SetSwapInterval( 1 ) < 0 ) 
        printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());

    /* clean up the screen */
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    SDL_GL_SwapWindow(mainWindow);

    return true;
}

void SetOpenGLSettings()
{
    // Set our OpenGL version.
    // SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // Turn on double buffering with a 24bit Z buffer.
    // You may need to change this to 16 or 32 for your system
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // This makes our buffer swap syncronized with the monitor's vertical refresh
    SDL_GL_SetSwapInterval(1);
}

void CheckSDLError(int line = -1)
{
    std::string error = SDL_GetError();

    /* TODO: Do something here */
    SDL_ClearError();
}

void Cleanup()
{
    SDL_GL_DeleteContext(mainContext);
    SDL_DestroyWindow(mainWindow);
    TTF_Quit();
    SDL_Quit();
}

/* Random helper */
void PrintSDLTTFVersion()
{
    const SDL_version *linkedVersion = TTF_Linked_Version();
    SDL_version compiledVersion;
    SDL_TTF_VERSION(&compiledVersion);

    std::cout << "Linked version:\n"
        << linkedVersion->major << "." << linkedVersion->minor << "." << linkedVersion->patch;

    std::cout << "Compiled version:\n"
        << compiledVersion.major << "." << compiledVersion.minor << "." << compiledVersion.patch
        << std::endl;

}
