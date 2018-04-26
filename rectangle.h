#ifndef __RECTANGLE__
#define __RECTANGLE__

#include "entity.h"
#include "game_memory.h"

#define RECT_SIZE 36
#define NUM_OF_RECT_CORNER 4
enum RectType
{
    REGULAR = 0,
    COLLISION = 1,
    HITBOX = 2,
    HURTBOX = 3
};

struct Rect
{
    Entity *entity;

    Vertex vertices[4];
    real32 width, height;

    v3 startingPosition;

#pragma warning(push)
#pragma warning(disable : 4201)
    union {
        struct
        {
            real32 maxX;
            real32 maxY;
        };
        real32 max[2];
    };

    union {
        struct
        {
            real32 minX;
            real32 minY;
        };
        real32 min[2];
    };
#pragma warning(pop)

    v3 topLeft;
    v3 topRight;
    v3 bottomLeft;
    v3 bottomRight;

    v4 color;

    v2 uvTextureTopLeft;
    v2 uvTextureTopRight;
    v2 uvTextureBottomLeft;
    v2 uvTextureBottomRight;

    bool isTextureUpsideDown;
    int size;
    RectType type;
    uint32 ttl; /* time to live / duration */
};

Rect *CreateRectangle(GameMemory *gm, Entity *entity, v3 startingPosition, v4 color,
                      real32 width, real32 height, bool isTraversable = true);
GLfloat *CreateDefaultRectangleVertices();

static GLuint g_rectIndices[] = {
    // Note that we start from 0!
    0, 1, 3, // First Triangle
    1, 2, 3  // Second Triangle
};

void CreateVertices(Rect *rect);

inline void DrawRectangle()
{
    const int totalIndiciesFromEbo = 6;
    glDrawElements(GL_TRIANGLES, totalIndiciesFromEbo, GL_UNSIGNED_INT, 0);
}

inline void DrawPointRectangle()
{
    glEnable(GL_PROGRAM_POINT_SIZE);
    const int totalIndiciesFromEbo = 6;
    glDrawElements(GL_POINTS, totalIndiciesFromEbo, GL_UNSIGNED_INT, 0);

    glDisable(GL_PROGRAM_POINT_SIZE);
}

inline void DrawDebugRectangle()
{
    const int totalIndiciesFromEbo = 6;
    glDrawElements(GL_POINTS, totalIndiciesFromEbo, GL_UNSIGNED_INT, 0);
}

#endif
