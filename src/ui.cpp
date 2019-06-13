#ifndef __UI__
#define __UI__

struct UIText
{
    const char *text;
    v2 basePosition;
    f32 scaleFactor;
};

struct UIInfo
{
    AABB range;
    v4 color;
    Bitmap *bitmap;
    b32 skipFilter;

    UIText uiText;
};

void PushToRenderGroup(
        RenderGroup *perFrameRenderGroup, GameMetadata *gameMetadata, GameMemory *gameMemory, UIText *u)
{
    PushStringRectToRenderGroup(
            perFrameRenderGroup, gameMetadata, gameMemory, V3(u->basePosition, 0), u->scaleFactor, u->text);
}

void PushToRenderGroup(
        RenderGroup *perFrameRenderGroup, GameMetadata *gameMetadata, GameMemory *gameMemory, UIInfo *u)
{
    v2 dim = u->range.halfDim * 2.0f;

    v3 startingPosition = V3(u->range.center, 0);
    Rect *rect = CreateRectangle(gameMemory, startingPosition, u->color, dim);
    rect->bitmapID = u->bitmap->bitmapID;
    rect->bitmap = u->bitmap;
    PushRenderGroupRectInfo(perFrameRenderGroup, rect, u->skipFilter);

    if (u->uiText.text != nullptr)
    {
        PushToRenderGroup(perFrameRenderGroup, gameMetadata, gameMemory, &u->uiText);
    }
}

#endif
