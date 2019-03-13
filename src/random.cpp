v2 V2(glm::vec3 A)
{
    v2 result;
    result.x = A.x;
    result.y = A.y;
    return result;

}

v3 V3(glm::vec3 A)
{
    v3 result;
    result.x = A.x;
    result.y = A.y;
    result.z = A.z;
    return result;

}

Rect *CreateMinimalRectInfo(GameMemory *gm, v2 min, v2 max)
{
    v2 dim = max - min;
    v2 xyPosition = min + (max - min) / 2.0f;
    v3 xyzPosition = {xyPosition.x, xyPosition.y, 0};
    Rect *r =  CreateRectangle(gm, xyzPosition, COLOR_RED, dim);
    return r;
}
/* I have no idea where to put these functions -- place holder area */
void AddDebugRect(GameMetadata *gm, Rect *rect)
{
    GameMemory *perFrameMemory = &gm->temporaryMemory;
    Rect *newRect = CreateMinimalRectInfo(perFrameMemory, rect->min, rect->max);

    newRect->bitmap = &gm->whiteBitmap;
    newRect->bitmapID = gm->whiteBitmap.bitmapID;
    newRect->renderLayer = DEBUG;

    PushBack(gm->rdaDebug, newRect);
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

