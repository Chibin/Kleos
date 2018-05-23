#include "rectangle.h"
#include "game_memory.h"

Rect *CreateRectangle(GameMemory *gm, v3 startingPosition, v4 color,
                      real32 width, real32 height)
{
    Rect *tmp = nullptr;
    tmp = static_cast<Rect *>(AllocateMemory(gm, sizeof(Rect)));
    ZeroSize(tmp, sizeof(Rect));

    tmp->startingPosition = startingPosition;
    tmp->color = color;
    tmp->width = width;
    tmp->height = height;
    tmp->size = RECT_SIZE;

    //tmp->spriteAnimation.UVCoords.UV[0] = v2{1, 1};
    //tmp->spriteAnimation.UVCoords.UV[1] = v2{1, 0};
    //tmp->spriteAnimation.UVCoords.UV[2] = v2{0, 0};
    //tmp->spriteAnimation.UVCoords.UV[3] = v2{0, 1};

    /* FIXME: This is starting the drawing from the origin, but not centered at
     * the origin
     */
    v2 min = { startingPosition.x, startingPosition.y };
    v2 max = { startingPosition.x + width, startingPosition.y + height };
    tmp->topRight = v3{ max.x, max.y, 0 };
    tmp->bottomRight = v3{ max.x, min.y, 0 };
    tmp->bottomLeft = v3{ min.x, min.y, 0 };
    tmp->topLeft = v3{ min.x, max.y, 0 };

    tmp->minX = min.x;
    tmp->minY = min.y;
    tmp->maxX = max.x;
    tmp->maxY = max.y;

    CreateVertices(tmp);

    return tmp;
}

void AssociateEntity(Rect *rect, Entity *entity, bool isTraversable)
{
    rect->entity = entity;
    rect->entity->position.x = rect->startingPosition.x;
    rect->entity->position.y = rect->startingPosition.y;
    rect->entity->position.z = rect->startingPosition.z;
    rect->entity->isTraversable = isTraversable;
    rect->entity->width = rect->width;
    rect->entity->height = rect->height;
    rect->entity->data = rect->vertices;
}

GLfloat *CreateDefaultRectangleVertices(GameMemory *gm)
{
    /*
     *   VertexCoords
     *   1,  1, 0,  Top Right
     *   1,  0, 0,  Bottom Right
     *   0,  0, 0,  Bottom Left
     *   0,  1, 0,  Top Left
     *
     *   TextureCoords
     *   1, 1,      Top Right
     *   1, 0,      Bottom Right
     *   0, 0,      Bottom Left
     *   0, 1       Top Left
     *
     *   Texture flipped
     *   1, 0,
     *   1, 1,
     *   0, 1,
     *   0, 0
     */

    GLfloat *vertices = nullptr;
    vertices = static_cast<GLfloat *>(AllocateMemory(gm, (sizeof(GLfloat) * 36)));

    vertices[0] = 1;
    vertices[1] = 1;
    vertices[2] = 0;

    vertices[3] = 0;
    vertices[4] = 0;
    vertices[5] = 0;
    vertices[6] = 0;

    vertices[7] = 1;
    vertices[8] = 1;

    vertices[9] = 1;
    vertices[10] = 0;
    vertices[11] = 0;

    vertices[12] = 0;
    vertices[13] = 0;
    vertices[14] = 0;
    vertices[15] = 0;

    vertices[16] = 1;
    vertices[17] = 0;

    vertices[18] = 0;
    vertices[19] = 0;
    vertices[20] = 0;

    vertices[21] = 0;
    vertices[22] = 0;
    vertices[23] = 0;
    vertices[24] = 0;

    vertices[25] = 0;
    vertices[26] = 0;

    vertices[27] = 0;
    vertices[28] = 1;
    vertices[29] = 0;

    vertices[30] = 0;
    vertices[31] = 0;
    vertices[32] = 0;
    vertices[33] = 0;

    vertices[34] = 0;
    vertices[35] = 1;

    return vertices;
}

void CreateVertices(Rect *rect)
{
    v3 normal = { 0.0f, 0.0f, 0.0f };
    /* texCoords + verticesCoords + color*/
    Vertex *vTopRight = &(rect->vertices[0]);

    vTopRight->vPosition = rect->topRight;
    vTopRight->vColor = rect->color;
    vTopRight->vNormal = normal;
    vTopRight->vUv = v2{ 1, 1 };

    Vertex *vBottomRight = &(rect->vertices[1]);
    vBottomRight->vPosition = rect->bottomRight;
    vBottomRight->vColor = rect->color;
    vBottomRight->vNormal = normal;
    vBottomRight->vUv = v2{ 1, 0 };

    Vertex *vBottomLeft = &(rect->vertices[2]);
    vBottomLeft->vPosition = rect->bottomLeft;
    vBottomLeft->vColor = rect->color;
    vBottomLeft->vNormal = normal;
    vBottomLeft->vUv = v2{ 0, 0 };

    Vertex *topLeft = &(rect->vertices[3]);
    topLeft->vPosition = rect->topLeft;
    topLeft->vColor = rect->color;
    topLeft->vNormal = normal;
    topLeft->vUv = v2{ 0, 1 };
}

inline Entity *GetEntity(Rect *rect)
{
    return rect->entity;
}

void PushRectVertex(GameMemory *gm, Rect *rect)
{
    /* We need 6 points because we need to create 2 triangles */
    const u8 numOfPoints = 6;
    ASSERT(gm->used + sizeof(Vertex) * numOfPoints <= gm->maxSize);

    auto *vertexPointer = (Vertex *)(gm->base + gm->used);

    for (memory_index i = 0; i < numOfPoints; i++)
    {
        memory_index index = g_rectIndices[i]; // NOLINT

        ASSERT(index < sizeof(rect->vertices));
        *(vertexPointer + i) = rect->vertices[index]; // NOLINT
    }

    gm->used += sizeof(Vertex) * numOfPoints;
}

void PushRectInfo(GameMemory *gm, Rect *rect)
{
    /* We need 6 points because we need to create 2 triangles */
    ASSERT(gm->used + sizeof(Rect) <= gm->maxSize);

    auto *rectPointer = (Rect *)(gm->base + gm->used);
    *rectPointer = *rect;
    gm->used += sizeof(Rect);
}

inline void UpdatePosition(Rect *r, v3 newPosition)
{
    v2 min = { newPosition.x, newPosition.y };
    v2 max = { newPosition.x + r->width, newPosition.y + r->height };

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
#if 0
    for (memory_index i = 0; i < NUM_OF_RECT_CORNER; i++)
    {
        r->vertices[i].vColor = color;
    }
#else
    for (auto &vertex : r->vertices)
    {
        vertex.vColor = color;
    }
#endif
}

inline void FlipXCoordinates(RectUVCoords *uv)
{
    v2 first = uv->UV[0];
    v2 second = uv->UV[1];
    v2 third = uv->UV[2];
    v2 fourth = uv->UV[3];

    uv->UV[0] = v2{ fourth.x, fourth.y };
    uv->UV[1] = v2{ third.x, third.y };
    uv->UV[2] = v2{ second.x, second.y };
    uv->UV[3] = v2{ first.x, first.y };
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
    return v2{ pixel.x / width, pixel.y / height };
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
    else if ((a->frameTTL = a->frameTTL - timeElapsed) <= 0)
    {
        /* TODO: There should be an assert that checks that we don't skip a minimum time frame */
        /* This could be replaced with a function later on */
        a->currentFrameIndex++;
        memory_index nextFrameOffset = a->currentFrameIndex % a->totalFrames;
        a->currentFrame = a->frameCoords + nextFrameOffset;
        ASSERT(a->currentFrame <= a->frameCoords + a->totalFrames);
        a->frameTTL = a->timePerFrame;
    }
}
