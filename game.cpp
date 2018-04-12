#include "game.h"
#include "entity.h"
#include "game_time.h"
#include "main.h"
#include "rectangle.h"
#include "shaders.h"
#include "texture.h"
#include <GL/glew.h>

#define ProcessOpenGLErrors() _processOpenGLErrors(__FILE__, __LINE__)

void _processOpenGLErrors(const char *file, int line);
void teststuff(GLuint &textureID);

void MainGameLoop(SDL_Window *mainWindow, RenderAPI &renderAPI)
{
    /* TODO: this is probably not the right spot for this */
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    bool continueRunning = true;

    GLuint program = CreateProgram("materials/programs/vertex.glsl",
                                   "materials/programs/fragment.glsl");
    GLuint debugProgram =
        CreateProgram("materials/programs/vertex.glsl",
                      "materials/programs/debug_fragment_shaders.glsl");

    /*  Each vertex attribute takes its data from memory managed by a
     *  VBO. VBO data -- one could have multiple VBOs -- is determined by the
     *  current VBO bound to GL_ARRAY_BUFFER when calling glVertexAttribPointer.
     *  Since the previously defined VBO was bound before
     *  calling glVertexAttribPointer vertex attribute 0 is now associated with
     * its vertex data.
     */

    GLuint vao, ebo, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &ebo);
    glGenBuffers(1, &vbo);
    Entity entity;
    GLuint *textureID;

    /* *entity, startingPosition, color, width, height, isTraversable */
    Rect *firstRect =
        CreateRectangle(&entity, v3{ 0, 0, 0 }, v4{ 0, 0, 0, 0 }, 1, 2, false);

#if 0
    TTF_Font *font = OpenFont();
    assert(font != NULL);
    textureID = StringToTexture(font, "testing this");
    firstRect->isTextureUpsideDown = true;
#else
    textureID = ImageToTexture("./materials/textures/awesomeface.png");
#endif

    CreateVertices(firstRect);

    OpenGLCreateVAO(vao, vbo, sizeof(Vertex) * NUM_OF_RECT_CORNER,
                    nullptr, /* use null as way to not load anything to vbo*/
                    ebo, sizeof(g_rectIndices), g_rectIndices);

    v2 screenResolution = { SCREEN_WIDTH, SCREEN_HEIGHT };
    GameTimestep *gameTimestep = nullptr;

    FindFile(GetProgramPath(), "render*dll");

    while (continueRunning)
    {
        continueRunning = ((renderAPI.updateAndRender)(
                               vao, vbo, *textureID, program, debugProgram,
                               screenResolution, &gameTimestep) != 0);

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
