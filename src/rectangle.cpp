#ifndef __RECTANGLE__
#define __RECTANGLE__
#include "rectangle.h"

inline void SetRectPoints(Rect *rect, v3 center, f32 width, f32 height)
{
    v2 centerXY = {0.5f * width, 0.5f * height};
    v2 basePositionXY = { center.x, center.y };
    v2 min = basePositionXY - centerXY;
    v2 max = basePositionXY + centerXY;

    rect->center = center;
    rect->dim.x = width;
    rect->dim.y = height;

#if 1
    /* This is for clock wise */
#else
    /* This is for ccw order*/
    rect->bottomRight = v3{ max.x, max.y, 0 };
    rect->topRight = v3{ max.x, min.y, 0 };
    rect->topLeft = v3{ min.x, min.y, 0 };
    rect->bottomLeft = v3{ min.x, max.y, 0 };
#endif
    rect->UV[0] = v2{ 1, 1 };
    rect->UV[1] = v2{ 1, 0 };
    rect->UV[2] = v2{ 0, 0 };
    rect->UV[3] = v2{ 0, 1 };

    rect->min = min;
    rect->max = max;
}

void SetRect(Rect *rect, v3 center, v4 color,
             real32 width, real32 height)
{
    ZeroSize(rect, sizeof(Rect));

    rect->color = color;
    rect->size = RECT_SIZE;

    /* FIXME: This is starting the drawing from the origin, but not centered at
     * the origin
     */
    SetRectPoints(rect, center, width, height);
}

Rect *CreateRectangle(GameMemory *gm, v3 center, v4 color,
                      real32 width, real32 height)
{
    Rect *tmp = nullptr;
    tmp = static_cast<Rect *>(AllocateMemory(gm, sizeof(Rect)));
    SetRect(tmp, center, color, width, height);

    return tmp;
}

Rect *CreateRectangle(GameMemory *gm, v3 center, v4 color, v2 rectDim)
{
    return CreateRectangle(gm, center, color, rectDim.x, rectDim.y);
}

Rect *CreateRectangle(GameMemory *gm, glm::vec3 center, v4 color, v2 rectDim)
{
    return CreateRectangle(
            gm,
            v3{center.x, center.y, center.z},
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

inline void CreateVertices(Rect *rect, Vertex *vertexPointer)
{
    const u8 numOfPoints = ARRAY_SIZE(g_rectIndices);
    union {
        struct
        {
            Vertex topRight;
            Vertex bottomRight;
            Vertex bottomLeft;
            Vertex topLeft;
        };
        Vertex vertices[4];
    };

    topRight.vPosition    = v3{ rect->max.x, rect->max.y, 0 };
    bottomRight.vPosition = v3{ rect->max.x, rect->min.y, 0 };
    bottomLeft.vPosition  = v3{ rect->min.x, rect->min.y, 0 };
    topLeft.vPosition     = v3{ rect->min.x, rect->max.y, 0 };

    topRight.vColor       = rect->color;
    bottomRight.vColor    = rect->color;
    bottomLeft.vColor     = rect->color;
    topLeft.vColor        = rect->color;

    v3 normal             = v3{ 0.0f, 0.0f, 0.0f };
    topRight.vNormal      = normal;
    bottomRight.vNormal   = normal;
    bottomLeft.vNormal    = normal;
    topLeft.vNormal       = normal;

    topRight.vUv          = rect->UV[0];
    bottomRight.vUv       = rect->UV[1];
    bottomLeft.vUv        = rect->UV[2];
    topLeft.vUv           = rect->UV[3];
    if(rect->frameDirection == RIGHT)
    {
        v2 first          = rect->UV[0];
        v2 second         = rect->UV[1];
        v2 third          = rect->UV[2];
        v2 fourth         = rect->UV[3];

        topRight.vUv      = v2{ fourth.x, fourth.y };
        bottomRight.vUv   = v2{ third.x, third.y };
        bottomLeft.vUv    = v2{ second.x, second.y };
        topLeft.vUv       = v2{ first.x, first.y };
    }

#if 1

    for (memory_index i = 0; i < numOfPoints; i++)
    {
        memory_index index = g_rectIndices[i]; // NOLINT

        ASSERT(index < sizeof(vertices));
        *(vertexPointer + i) = vertices[index]; // NOLINT
    }
#endif
}

void PushRectVertex(GameMemory *gm, Rect *rect)
{
    /* We need 6 points because we need to create 2 triangles */
    const u8 numOfPoints = 6;
    ASSERT(gm->used + sizeof(Vertex) * numOfPoints <= gm->maxSize);

    auto *vertexPointer = (Vertex *)(gm->base + gm->used);

#if 1
    CreateVertices(rect, vertexPointer);
#else
    for (memory_index i = 0; i < numOfPoints; i++)
    {
        memory_index index = g_rectIndices[i]; // NOLINT

        ASSERT(index < sizeof(rect->vertices));
        *(vertexPointer + i) = rect->vertices[index]; // NOLINT
    }
#endif

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
    v2 center = 0.5f * r->dim;

#if 0
    v2 min = { newPosition.x, newPosition.y };
    v2 max = { newPosition.x + r->width, newPosition.y + r->height };
#else
    v2 basePositionXY = { newPosition.x, newPosition.y };
    v2 min = basePositionXY - center;
    v2 max = basePositionXY + center;
#endif

    r->center = newPosition;

    r->min = min;
    r->max = max;
}

inline void UpdateUV(Rect *r, RectUVCoords uv)
{
    r->UV[0] = uv.UV[0];
    r->UV[1] = uv.UV[1];
    r->UV[2] = uv.UV[2];
    r->UV[3] = uv.UV[3];
}

inline void UpdateColors(Rect *r, v4 color)
{
#if 0
    for (memory_index i = 0; i < NUM_OF_RECT_CORNER; i++)
    {
        r->vertices[i].vColor = color;
    }
#else
    r->color = color;
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
