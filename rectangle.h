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

    memory_index bitmapID;
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

inline void DrawRawRectangle(memory_index count)
{
    const u32 numOfPoints = 6;
    u32 totalVertices = SafeCastToU32(count) * numOfPoints;
    glDrawArrays(GL_TRIANGLES, 0, totalVertices);
}

inline void DrawRawPointRectangle(memory_index count)
{
    const u32 numOfPoints = 6;
    u32 totalVertices = SafeCastToU32(count) * numOfPoints;
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDrawArrays(GL_POINTS, 0, totalVertices);
    glDisable(GL_PROGRAM_POINT_SIZE);
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

struct RenderGroup
{
    GameMemory vertexMemory;
    memory_index rectCount;
};

inline void PushRect(GameMemory *gm, Rect *rect)
{
    /* We need 6 points because we need to create 2 triangles */
    const u8 numOfPoints = 6;
    ASSERT(gm->used + sizeof(Vertex) * numOfPoints <= gm->maxSize);

    Vertex *vertexPointer = (Vertex *) (gm->base + gm->used);

    for (memory_index i = 0; i < numOfPoints; i++)
    {
        memory_index index = g_rectIndices[i];
        *(vertexPointer+i) = rect->vertices[index];
    }

    gm->used += sizeof(Vertex) * numOfPoints;
}

inline void UpdatePosition(Rect *r, v3 newPosition)
{
    v2 min = {newPosition.x, newPosition.y};
    v2 max = {newPosition.x + r->width, newPosition.y + r->height};

    v3 topRight = v3{ max.x, max.y, 0 };
    v3 bottomRight = v3{ max.x, min.y, 0 };
    v3 bottomLeft = v3{ min.x, min.y, 0 };
    v3 topLeft = v3{ min.x, max.y, 0 };

    r->vertices[0].vPosition = topRight;
    r->vertices[1].vPosition = bottomRight;
    r->vertices[2].vPosition = bottomLeft;
    r->vertices[3].vPosition = topLeft;
}

inline void UpdateColors(Rect *r, v4 color)
{
    for (memory_index i = 0; i < NUM_OF_RECT_CORNER; i++)
    {
        r->vertices[i].vColor = color;
    }
}

inline void PushRect(RenderGroup *rg, Rect *rect)
{
    PushRect(&rg->vertexMemory, rect);
    rg->rectCount++;
}
#endif
