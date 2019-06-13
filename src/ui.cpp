#ifndef __UI__
#define __UI__

#include "ui.h"

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

UIInfo *CreateSelectButtonUI(GameMetadata *gameMetadata)
{
    UIInfo *uiInfo = (UIInfo *)AllocateMemory0(&gameMetadata->reservedMemory, sizeof(UIInfo));

    AABB selectedUI = {};
    selectedUI.halfDim = v2{0.25, 1.0f};
    selectedUI.center = v2{0.75f, 0.0f};

    uiInfo->range = selectedUI;
    uiInfo->color = COLOR_BLACK - TRANSPARENCY(0.6f);
    uiInfo->bitmap = &gameMetadata->whiteBitmap;
    uiInfo->skipFilter = true;

    return uiInfo;
}

UIInfo *CreateCommandPromptUI(GameMetadata *gameMetadata)
{

    UIInfo *uiInfo = (UIInfo *)AllocateMemory0(&gameMetadata->reservedMemory, sizeof(UIInfo));

    f32 padding = 0.05f;
    f32 scaleFactor = 0.50f;
    f32 screenHeight = gameMetadata->screenResolution.v[1];
    f32 scale = 0.50f;
    f32 rectHeight = gameMetadata->fontBitmap.height / screenHeight * scale;

    uiInfo->range.halfDim.x = 1;
    uiInfo->range.halfDim.y = rectHeight * 0.5f;
    uiInfo->range.center = v2{ 0.0f, -0.35f + rectHeight * 0.5f };
    uiInfo->color = COLOR_BLACK - TRANSPARENCY(0.6f);
    uiInfo->bitmap = &gameMetadata->whiteBitmap;
    uiInfo->skipFilter = true;

    uiInfo->uiText.basePosition = v2{ -1 + padding, -0.35f + rectHeight * 0.5f };
    uiInfo->uiText.scaleFactor = scaleFactor;
    ASSERT(gameMetadata->editMode.commandPrompt != nullptr);
    uiInfo->uiText.text = gameMetadata->editMode.commandPrompt;

    return uiInfo;
}

UIInfo *CreateEditUIBar(GameMetadata *gameMetadata)
{
    f32 screenHeight = gameMetadata->screenResolution.v[1];

    f32 scaleFactor = 0.50f;
    f32 padding = 0.05f;
    f32 rectHeight = gameMetadata->fontBitmap.height / screenHeight * scaleFactor;

    UIInfo *uiInfo = (UIInfo *)AllocateMemory0(&gameMetadata->reservedMemory, sizeof(UIInfo));
    uiInfo->range.center = v2{ 0.0f, -1 + rectHeight * 0.5f };
    uiInfo->range.halfDim.x = 1;
    uiInfo->range.halfDim.y = rectHeight * 0.5f;
    uiInfo->color = COLOR_BLACK - TRANSPARENCY(0.5f);
    uiInfo->bitmap = &gameMetadata->whiteBitmap;
    uiInfo->skipFilter = true;
    uiInfo->uiText.basePosition = v2{ -1 + padding, -1 + rectHeight * 0.5f };
    uiInfo->uiText.scaleFactor = scaleFactor;
    uiInfo->uiText.text = "Edit";

    return uiInfo;
}

void SetUI(GameMetadata *gameMetadata)
{
    UIInfo *uiInfo = CreateSelectButtonUI(gameMetadata);
    HashAdd(gameMetadata->hashCharUIInfo, "select_rect_ui", uiInfo);

    uiInfo = CreateCommandPromptUI(gameMetadata);
    HashAdd(gameMetadata->hashCharUIInfo, "command_prompt_ui", uiInfo);

    uiInfo = CreateEditUIBar(gameMetadata);
    HashAdd(gameMetadata->hashCharUIInfo, "edit_ui_bar", uiInfo);

}

#endif
