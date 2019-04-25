#ifndef __DEBUG_HELPER__
#define __DEBUG_HELPER__

/* I have no idea where to put these functions -- place holder area */
void AddDebugRect(GameMetadata *gm, Rect *rect, v4 color)
{
    GameMemory *perFrameMemory = &gm->temporaryMemory;
    Rect *newRect = CreateMinimalRectInfo(perFrameMemory, color, rect->min, rect->max);

    newRect->bitmap = &gm->whiteBitmap;
    newRect->bitmapID = gm->whiteBitmap.bitmapID;
    newRect->renderLayer = DEBUG;

    PushBack(gm->rdaDebug, newRect);
}

void AddDebugRect(GameMetadata *gm, Rect *rect)
{
    AddDebugRect(gm, rect, COLOR_RED);
}

void AddDebugRectUI(GameMetadata *gm, MinMax *minMax, v4 color)
{
    GameMemory *perFrameMemory = &gm->temporaryMemory;
    Rect *newRect = CreateMinimalRectInfo(perFrameMemory, color, minMax->min, minMax->max);

    newRect->bitmap = &gm->whiteBitmap;
    newRect->bitmapID = gm->whiteBitmap.bitmapID;
    newRect->renderLayer = DEBUG;

    PushBack(gm->rdaDebugUI, newRect);
}

void AddDebugRect(GameMetadata *gm, MinMax *minMax, v4 color)
{
    GameMemory *perFrameMemory = &gm->temporaryMemory;
    Rect *newRect = CreateMinimalRectInfo(perFrameMemory, color, minMax->min, minMax->max);

    newRect->bitmap = &gm->whiteBitmap;
    newRect->bitmapID = gm->whiteBitmap.bitmapID;
    newRect->renderLayer = DEBUG;

    PushBack(gm->rdaDebug, newRect);
}

void AddDebugRect(GameMetadata *gm, AABB *aabb, v4 color)
{
    MinMax minMax = {};
    GetMinMax(aabb, &minMax);
    AddDebugRect(gm, &minMax, color);
}

void AddDebugRectUI(GameMetadata *gm, AABB *aabb, v4 color)
{
    GameMemory *perFrameMemory = &gm->temporaryMemory;
    Rect *newRect = CreateMinimalRectInfo(perFrameMemory, color, aabb);

    newRect->bitmap = &gm->whiteBitmap;
    newRect->bitmapID = gm->whiteBitmap.bitmapID;
    newRect->renderLayer = DEBUG;

    PushBack(gm->rdaDebugUI, newRect);
}

void AddDebugRect(GameMetadata *gm, FrameState *frameState, v3 position, v4 color)
{
    GameMemory *perFrameMemory = &gm->temporaryMemory;
    Rect *newRect = CreateFrameRect(perFrameMemory, frameState, position, color);

    newRect->bitmap = &gm->whiteBitmap;
    newRect->bitmapID = gm->whiteBitmap.bitmapID;
    newRect->renderLayer = DEBUG;

    PushBack(gm->rdaDebug, newRect);
}

#endif
