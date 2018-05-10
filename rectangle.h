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

struct RectUVCoords {
#pragma warning(push)
#pragma warning(disable : 4201)
    union {
        struct {
            v2 topRight;
            v2 bottomRight;
            v2 bottomLeft;
            v2 topLeft;
        };
        v2 UV[4];
#pragma warning(pop)
    };
};

enum Direction {
    LEFT = 1,
    RIGHT = 2,
};

struct Animation2D
{
    RectUVCoords *frameCoords;
    u32 totalFrames;
    /* May be have a function pointer or some type of function to determine how
     * long each frames last.
     * Or a waypoint curve?
     */ 
    f32 timePerFrame;
    f32 frameTTL;
    RectUVCoords *currentFrame;
    memory_index currentFrameIndex;
    Direction direction;
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

    Animation2D *sprites;
    u32 totalSprites;
    Direction frameDirection;

    bool isTextureUpsideDown;
    int size;
    RectType type;
    uint32 ttl; /* time to live / duration */
};

Rect *CreateRectangle(GameMemory *gm, v3 startingPosition, v4 color, real32 width, real32 height);
GLfloat *CreateDefaultRectangleVertices();
void CreateVertices(Rect *rect);
void AssociateEntity(Rect *rect, Entity *entity, bool isTraversable);

static GLuint g_rectIndices[] = {
    // Note that we start from 0!
    0, 1, 3, // First Triangle
    1, 2, 3  // Second Triangle
};

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

inline void UpdateUV(Rect *r, RectUVCoords uv)
{
    r->vertices[0].vUv = uv.UV[0];
    r->vertices[1].vUv = uv.UV[1];
    r->vertices[2].vUv = uv.UV[2];
    r->vertices[3].vUv = uv.UV[3];
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

inline void FlipXCoordinates(RectUVCoords *uv)
{
    v2 first  = uv->UV[0];
    v2 second = uv->UV[1];
    v2 third  = uv->UV[2];
    v2 fourth = uv->UV[3];

    uv->UV[0] = v2{fourth.x, fourth.y};
    uv->UV[1] = v2{third.x, third.y};
    uv->UV[2] = v2{second.x, second.y};
    uv->UV[3] = v2{first.x, first.y};
}

inline void FlipYAxis(RectUVCoords *uv)
{
    FlipXCoordinates(uv);
}

inline void FlipYAxisOnAllFrames(Animation2D *a)
{
    FlipYAxis(a->frameCoords + 0);
    FlipYAxis(a->frameCoords + 1);
}

inline v2 PixelToUV(v2 pixel, u32 width, u32 height)
{
    return v2{pixel.x / width, pixel.y / height};
}

inline void UpdateFrameDirection(Animation2D *a, Direction d)
{
    if (a->direction == d)
    {
        return;
    }

    FlipYAxisOnAllFrames(a);
    a->direction = d;
}

inline void UpdateCurrentFrame(Animation2D *a, f32 timeElapsed)
{
    if (a->currentFrame == nullptr)
    {
        ASSERT(a->frameCoords);
        a->currentFrame = &a->frameCoords[0];
        a->currentFrameIndex = 0;
        a->frameTTL = a->timePerFrame;
    }
    else if ( (a->frameTTL = a->frameTTL - timeElapsed ) <= 0)
    {
        /* TODO: There should be an assert that checks that we don't skip a minimum time frame */
        /* This could be replaced with a function later on */
        a->currentFrameIndex++;
        memory_index nextFrameOffset = a->currentFrameIndex  % a->totalFrames;
        a->currentFrame = a->frameCoords + nextFrameOffset ;
        ASSERT(a->currentFrame <= a->frameCoords + a->totalFrames);
        a->frameTTL = a->timePerFrame;
    }
}
#endif
