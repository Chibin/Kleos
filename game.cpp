#ifndef __GAME__
#define __GAME__

#include "input.cpp"

/* should already be included in the main.cpp */
#include "shaders.cpp"


#define ProcessOpenGLErrors() _processOpenGLErrors(__FILE__, __LINE__)
 
void _processOpenGLErrors(const char *file, int line);
void teststuff(GLuint &textureID);

typedef void (__cdecl *RENDER)(GLuint, GLuint, GLuint, GLuint); 

void MainGameLoop(SDL_Window *mainWindow)
{
    /* TODO: this is probably not the right spot for this */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    bool continueRunning = true;

    GLuint program = CreateProgram("materials/programs/vertex.glsl",
                                 "materials/programs/fragment.glsl");
    GLuint debugProgram = CreateProgram("materials/programs/vertex.glsl",
            "materials/programs/debug_fragment_shaders.glsl");

    GLfloat points[12] = { -0.5, -0.5,
                           0.5, -0.5,
                           0.5, 0.5,
                           0.5, 0.5,
                           -0.5, 0.5,
                           -0.5, -0.5 };

#define PRINTFONT 1
#if PRINTFONT
    /* Flip the texture coordinates if we're using SDL TTF. OpenGL normally
     * draws things upsidedown
     */
    GLfloat vertices[] = {
        // Positions           // Texture Coords
        0.5f,  0.5f, 0.0f,     1.0f, 0.0f,   // Top Right
        0.5f, -0.5f, 0.0f,     1.0f, 1.0f,   // Bottom Right
        -0.5f, -0.5f, 0.0f,    0.0f, 1.0f,   // Bottom Left
        -0.5f,  0.5f, 0.0f,    0.0f, 0.0f    // Top Left 
    };
#else
    GLfloat vertices[] = {
        // Positions           // Texture Coords
        0.5f,  0.5f, 0.0f,     1.0f, 1.0f,   // Top Right
        0.5f, -0.5f, 0.0f,     1.0f, 0.0f,   // Bottom Right
        -0.5f, -0.5f, 0.0f,    0.0f, 0.0f,   // Bottom Left
        -0.5f,  0.5f, 0.0f,    0.0f, 1.0f    // Top Left 
    };
#endif

    GLuint indices[] = {  // Note that we start from 0!
        0, 1, 3,   // First Triangle
        1, 2, 3    // Second Triangle
    };  

    GLuint indices2[] = {  // Note that we start from 0!
        1, 2, 3    // Second Triangle
    };  


    /*  Each vertex attribute takes its data from memory managed by a
     *  VBO. VBO data -- one could have multiple VBOs -- is determined by the
     *  current VBO bound to GL_ARRAY_BUFFER when calling glVertexAttribPointer.
     *  Since the previously defined VBO was bound before
     *  calling glVertexAttribPointer vertex attribute 0 is now associated with its
     *  vertex data. 
     */

    GLuint vao, ebo, vbo;
    glGenVertexArrays(1, &vao);  
    glGenBuffers(1, &ebo);  
    glGenBuffers(1, &vbo);  

    /*  Initialization code (done once (unless your object frequently changes)) */
    // 1. Bind Vertex Array Object
    glBindVertexArray(vao);
        // 2. Copy our vertices array in a buffer for OpenGL to use

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3* sizeof(GLfloat)));

    /*5. Unbind the VAO (NOT THE EBO). We need to make sure that we always unbind,
     * otherwise we might accidentally save some unwatned commands into
     * the vertext array object
     */
    /*  It is common practice to unbind OpenGL objects when we're done
     *  configuring them so we don't mistakenly (mis)configure them
     *  elsewhere. */
    glBindVertexArray(0);

    GLuint textureID;
    glGenTextures(1, &textureID);

    TTF_Font *font = OpenFont();
    assert(font != NULL);

#if PRINTFONT
    StringToTexture(textureID, font, "testing this");
#else
    ImageToTexture(textureID, "./materials/textures/awesomeface.png");
#endif

    /* Load library */
    HMODULE RenderDLL;

    RenderDLL = LoadLibrary("render.dll");
    if(!RenderDLL)
        printf("Failed to load library! \n");

    RENDER Render;
    Render = (RENDER)GetProcAddress(RenderDLL, "Render");

    if(!Render)
        printf("Failed to load function \"Render\"!\n");

    while (continueRunning)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                continueRunning = false;

            if (event.type == SDL_KEYDOWN)
                InputHandler(event.key.keysym.sym, &continueRunning, mainWindow);
        }

        /* TODO: One time init might be done here as the game progress ? */

        /* start with an 'empty' canvas */
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ProcessOpenGLErrors();

        //GameUpdate();
        (Render)(vao, textureID, program, debugProgram);

        /* equivalent to glswapbuffer? */
        SDL_GL_SwapWindow(mainWindow);
    }
}

void _processOpenGLErrors(const char *file, int line)
{
    // Process/log the error.
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR){
        std::string error;

        printf("error detected at %s:%d:\n", file, line);

        switch(err) {
            case GL_INVALID_OPERATION: error="INVALID_OPERATION"; break;
            case GL_INVALID_ENUM:      error="INVALID_ENUM";      break;
            case GL_INVALID_VALUE:     error="INVALID_VALUE";     break;
            case GL_OUT_OF_MEMORY:     error="OUT_OF_MEMORY";     break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                                       error="INVALID_FRAMEBUFFER_OPERATION";  break;
            default: printf("something bad happened. "
                            "unknown error: %d\n", err); break;
        }

        printf("something bad happened: %s\n", error.c_str());
    }
}

#endif
