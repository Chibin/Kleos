#ifndef __GAME__
#define __GAME__

#include "input.cpp"

/* should already be included in the main.cpp */
#include "shaders.cpp"

void ProcessOpenGLErrors();
void teststuff(GLuint &textureID);

void MainGame(SDL_Window *mainWindow)
{
    /* TODO: this is probably not the right spot for this */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    bool continueRunning = true;

    GLuint program = LoadShaders("fixtures/vertex.glsl",
                                 "fixtures/fragment.glsl");

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

#if PRINTFONT
    TTF_Font *font = OpenFont();
    assert(font != NULL);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    SDL_Surface *surface = NULL;
    surface = RenderText(font, "testing this");
    assert(surface != NULL);

    int mode = GL_RGB;
    if(surface->format->BytesPerPixel == 4)
        mode = GL_RGBA;

    /* https://www.khronos.org/opengl/wiki/Common_Mistakes
     * Creating a complete texture
     */
    glTexImage2D(GL_TEXTURE_2D, 0, mode, surface->w, surface->h, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);

    glTexParameteri(GL_TEXTURE_2D,  GL_GENERATE_MIPMAP, GL_TRUE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glUniform1i(glGetUniformLocation(program, "tex1"), 0);

    //Clean up the surfaceace and font
    SDL_FreeSurface(surface);
    glBindTexture(GL_TEXTURE_2D, 0);
#else
    imageToTexture(textureID, "awesomeface.png");
#endif


    while (continueRunning)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
                continueRunning = false;

            if (event.type == SDL_KEYDOWN)
                HandleInput(event.key.keysym.sym, &continueRunning, mainWindow);
        }

        /* start with an 'empty' canvas */
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ProcessOpenGLErrors();

        /* Render graphics */
        glUseProgram(program);

        glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);   // We want to repeat this pattern so we set kept it at GL_REPEAT
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);   // We want to repeat this pattern so we set kept it at GL_REPEAT
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glUniform1i(glGetUniformLocation(program, "tex"), 0);

        glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        /* equivalent to glswapbuffer? */
        SDL_GL_SwapWindow(mainWindow);
    }
}

void ProcessOpenGLErrors()
{
    // Process/log the error.
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR){
        std::string error;

        switch(err) {
            case GL_INVALID_OPERATION:              error="INVALID_OPERATION";              break;
            case GL_INVALID_ENUM:                   error="INVALID_ENUM";                   break;
            case GL_INVALID_VALUE:                  error="INVALID_VALUE";                  break;
            case GL_OUT_OF_MEMORY:                  error="OUT_OF_MEMORY";                  break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
            default: printf("something bad happened. unknown error: %d\n", err); break;
        }

        printf("something bad happened: %s\n", error.c_str());

    }
}

#endif
