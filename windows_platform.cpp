#ifndef __WINDOWS_PLATFORM__
#define __WINDOWS_PLATFORM__

typedef bool (__cdecl *UPDATEANDRENDER)(GLuint vao, GLuint vbo, GLuint textureID, GLuint program, GLuint debugProgram, v2 screenResolution, GLfloat *vertices, GameTimestep **gameTimestep);

struct RenderAPI {
    std::string libraryName;
    HMODULE renderDLL;
    UPDATEANDRENDER updateAndRender;
};

void _setOpenGLSettings();
void CheckSDLError(int);
void WindowsCleanup();
bool WindowSetup();
bool WindowsOpenGLSetup();
bool WindowsSDLTTFSetup();

/* functions related to windows specific platform */
bool WindowSetup()
{
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

    return true;
}

bool WindowsSDLTTFSetup()
{
    if (TTF_Init() < 0) {
        printf("TTF failed to init\n");
        return false;
    }

    printf("TTF init complete\n");

    return true;
}

bool WindowsOpenGLSetup()
{
    _setOpenGLSettings();
    /* create our opengl context and attach it to our window */
    mainContext = SDL_GL_CreateContext(mainWindow);

    /* initialize to start using opengl */
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();

    GLenum err;
    if ((err = glGetError()) != GL_NO_ERROR) {
        /* Ignore GL_INVALID_ENUM. There are cases where using glewExperimental
         * can cause a GL_INVALID_ENUM, which is fine -- just ignore it.
         * Otherwise, we do have a problem.
         */
        if ( err != GL_INVALID_ENUM )
            printf("OpenGL: found true error x%x\n", err);
    }

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

void _setOpenGLSettings()
{
    // Set our OpenGL version.
    // SDL_GL_CONTEXT_CORE gives us only the newer version, deprecated functions are disabled
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

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

void WindowsCleanup()
{
    SDL_GL_DeleteContext(mainContext);
    SDL_DestroyWindow(mainWindow);
    TTF_Quit();
    SDL_Quit();
}

bool LoadDLLWindows(RenderAPI *renderAPI)
{
    renderAPI->renderDLL = LoadLibrary("render.dll");
    if(!renderAPI->renderDLL) {
        printf("Failed to load library! \n");
        return false;
    }

    HMODULE RenderDLL = renderAPI->renderDLL;
    renderAPI->updateAndRender = (UPDATEANDRENDER)GetProcAddress(RenderDLL,
            "UpdateAndRender");

    if(!renderAPI->updateAndRender) {
        printf("Failed to load function \"UpdateAndRender\"!\n");
        return false;
    }
    return true;
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
#endif
