#ifndef __RECTANGLE__
#define __RECTANGLE__
#include "rectangle.h"

inline void SetRectPoints(Rect *rect, v3 basePosition, f32 width, f32 height)
{
    v2 center = {0.5f * width, 0.5f * height};
#if 0
    v2 min = { basePosition.x, basePosition.y };
    v2 max = { basePosition.x + width, basePosition.y + height };
#else
    v2 basePositionXY = { basePosition.x, basePosition.y };
    v2 min = basePositionXY - center;
    v2 max = basePositionXY + center;

#endif
    rect->basePosition = basePosition;
    rect->width = width;
    rect->height = height;

#if 1
    /* This is for clock wise */
    rect->topRight = v3{ max.x, max.y, 0 };
    rect->bottomRight = v3{ max.x, min.y, 0 };
    rect->bottomLeft = v3{ min.x, min.y, 0 };
    rect->topLeft = v3{ min.x, max.y, 0 };
#else
    /* This is for ccw */
    rect->bottomRight = v3{ max.x, max.y, 0 };
    rect->topRight = v3{ max.x, min.y, 0 };
    rect->topLeft = v3{ min.x, min.y, 0 };
    rect->bottomLeft = v3{ min.x, max.y, 0 };
#endif

    rect->min = min;
    rect->max = max;
}

void SetRect(Rect *rect, v3 basePosition, v4 color,
             real32 width, real32 height)
{
    ZeroSize(rect, sizeof(Rect));

    rect->color = color;
    rect->size = RECT_SIZE;

    /* FIXME: This is starting the drawing from the origin, but not centered at
     * the origin
     */
    SetRectPoints(rect, basePosition, width, height);
    CreateVertices(rect);
}

Rect *CreateRectangle(GameMemory *gm, v3 basePosition, v4 color,
                      real32 width, real32 height)
{
    Rect *tmp = nullptr;
    tmp = static_cast<Rect *>(AllocateMemory(gm, sizeof(Rect)));
    SetRect(tmp, basePosition, color, width, height);

    return tmp;
}

Rect *CreateRectangle(GameMemory *gm, v3 basePosition, v4 color, v2 rectDim)
{
    return CreateRectangle(gm, basePosition, color, rectDim.x, rectDim.y);
}

Rect *CreateRectangle(GameMemory *gm, glm::vec3 basePosition, v4 color, v2 rectDim)
{
    return CreateRectangle(
            gm,
            v3{basePosition.x, basePosition.y, basePosition.z},
            color,
            rectDim.x,
            rectDim.y);
}

Rect *CreateMinimalRectInfo(GameMemory *gm, v4 color, v2 min, v2 max)
{
    v2 dim = max - min;
    v2 xyPosition = min + (max - min) / 2.0f;
    v3 xyzPosition = {xyPosition.x, xyPosition.y, 0};
    Rect *r =  CreateRectangle(gm, xyzPosition, color, dim);
    return r;
}

Rect *CreateMinimalRectInfo(GameMemory *gm, v4 color, AABB *aabb)
{
    v2 dim = aabb->halfDim * 2.0f;
    v3 xyzPosition = {aabb->center.x, aabb->center.y, 0};
    Rect *r =  CreateRectangle(gm, xyzPosition, color, dim);
    return r;
}

Rect *CreateMinimalRectInfo(GameMemory *gm, v2 min, v2 max)
{
    return CreateMinimalRectInfo(gm, COLOR_RED, min, max);
}


void AssociateEntity(Rect *rect, Entity *entity, bool isTraversable)
{
    rect->entity = entity;
    rect->entity->position.x = rect->basePosition.x;
    rect->entity->position.y = rect->basePosition.y;
    rect->entity->position.z = rect->basePosition.z;
    rect->entity->isTraversable = isTraversable;
    rect->entity->width = rect->width;
    rect->entity->height = rect->height;
    rect->entity->data = rect->vertices;
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
    v2 center = 0.5f * v2{r->width, r->height};

#if 0
    v2 min = { newPosition.x, newPosition.y };
    v2 max = { newPosition.x + r->width, newPosition.y + r->height };
#else
    v2 basePositionXY = { newPosition.x, newPosition.y };
    v2 min = basePositionXY - center;
    v2 max = basePositionXY + center;
#endif

    //r->basePosition = newPosition;

#if 1
    /* This is for clockwise */
    v3 topRight = v3{ max.x, max.y, 0 };
    v3 bottomRight = v3{ max.x, min.y, 0 };
    v3 bottomLeft = v3{ min.x, min.y, 0 };
    v3 topLeft = v3{ min.x, max.y, 0 };
#else
    /* This is for counter clockwise */
    v3 bottomRight = v3{ max.x, max.y, 0 };
    v3 topRight = v3{ max.x, min.y, 0 };
    v3 topLeft = v3{ min.x, min.y, 0 };
    v3 bottomLeft = v3{ min.x, max.y, 0 };
#endif

    r->vertices[0].vPosition = topRight;
    r->vertices[1].vPosition = bottomRight;
    r->vertices[2].vPosition = bottomLeft;
    r->vertices[3].vPosition = topLeft;

    r->min = min;
    r->max = max;
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
    memory_index i = a->totalFrames;
    while(i--) FlipYAxis(a->frameCoords + i);
}

inline v2 PixelToUV(v2 pixel, u32 width, u32 height)
{
    return v2{ pixel.x / width, pixel.y / height };
}

inline v2 PixelToUV(v2i pixel, u32 width, u32 height)
{
    return v2{ (float)pixel.x / (float)width, (float)pixel.y / (float)height };
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
        a->currentFrameIndex = nextFrameOffset;
        a->currentFrame = a->frameCoords + nextFrameOffset;
        ASSERT(a->currentFrame <= a->frameCoords + a->totalFrames);
        a->frameTTL = a->timePerFrame;
    }
}
#endif
