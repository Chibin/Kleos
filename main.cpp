
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <stdio.h>
#include <stdlib.h> /* for calloc */
#include <assert.h>
#include <windows.h>

#include <SDL.h>
#include <SDL_ttf.h>

#define GL3_PROTOTYPES 1
#include <GL/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>

#pragma warning(push)
#pragma warning (disable: 4201)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>
#pragma warning(pop)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "math.cpp"

#include "logger.cpp"
#include "font.cpp"
#include "texture.cpp"
#include "shaders.cpp"

#include "entity.cpp"
#include "camera.cpp"
#include "input.cpp"

#include "rectangle.cpp"

// screen dimension constants
#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768

std::string programName = "First game trial :)";
static SDL_Window *mainWindow;
/* our opengl context handle */
static SDL_GLContext mainContext;

#include "windows_platform.cpp"
/* different platforms may have different ways to load a library, so creating a
 * struct within the platform file 
 * Not sure about the name though.
 */
static RenderAPI renderAPI;

#include "game.cpp"

int main(int argc, char *argv[])
{
    if(!WindowSetup() || 
            !WindowsOpenGLSetup() || 
            !WindowsSDLTTFSetup() ||
            !LoadDLLWindows(&renderAPI))
        return -1;

    MainGameLoop(mainWindow);
    WindowsCleanup();

    return 0;
}

