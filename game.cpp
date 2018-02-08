#ifndef __GAME__
#define __GAME__

#include "input.cpp"

/* should already be included in the main.cpp */
#include "shaders.cpp"

void ProcessOpenGLErrors();

void MainGame(SDL_Window *mainWindow)
{
    bool continueRunning = true;

    GLuint program = LoadShaders("fixtures/vertex.glsl",
                                 "fixtures/fragment.glsl");

    GLfloat points[12] = { -0.5, -0.5,
                           0.5, -0.5,
                           0.5, 0.5,
                           0.5, 0.5,
                           -0.5, 0.5,
                           -0.5, -0.5 };

    GLuint vao, buffer;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);

    glUseProgram(program);

    GLuint loc = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, 0, 0);

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
        glDrawArrays(GL_TRIANGLES, 0, 12);

        /* equivalent to glswapbuffer? */
        SDL_GL_SwapWindow(mainWindow);
    }
}


void ProcessOpenGLErrors()
{
    // Process/log the error.
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
        printf("something bad happened:%d\n", err);
}

#endif
