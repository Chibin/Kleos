#ifndef __RENDER__
#define __RENDER__

void DrawScene(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        VulkanContext *vc,
        VulkanBuffers *vkBuffers,
        Camera *camera,
        glm::mat4 *projection,
        RectDynamicArray *hitBoxes,
        RectDynamicArray *hurtBoxes)
{
    const b32 skipFilter = false;
    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;

    ASSERT(g_rectManager->Traversable.rda.size > 0);

    RectDynamicArray *newWorldObjects = CreateRDAForNewWorldObjects(gameMetadata);

    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, &g_rectManager->Traversable.rda, skipFilter);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, &g_rectManager->NonTraversable.rda, skipFilter);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, newWorldObjects, skipFilter);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, hitBoxes, skipFilter);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, hurtBoxes, skipFilter);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, gameMetadata->rdaDebug, skipFilter);

    PushRenderGroupRectInfo(perFrameRenderGroup,
            HashGetValue(HashEntityRect, gameMetadata->hashEntityRect, gameMetadata->playerEntity), skipFilter);
    Rect *minimalRect = CreateMinimalRectInfo(perFrameMemory, g_enemyNPC);
    UpdateNPCAnimation(g_enemyNPC, minimalRect);
    PushRenderGroupRectInfo(perFrameRenderGroup, minimalRect, skipFilter);

    ClearUsedVertexRenderGroup(perFrameRenderGroup);

#if 1
    Rect **sortedRectInfo = MergeSortRenderGroupRectInfo(perFrameRenderGroup, perFrameMemory);
#else
    QuickSortRenderGroupRectInfo(perFrameRenderGroup);
#endif

    UpdateUBOandPushConstants(
            gameMetadata,
            vc,
            camera,
            projection);

    DrawRenderGroup(
            gameMetadata,
            perFrameRenderGroup,
            sortedRectInfo,
            vc,
            vkBuffers);

    ClearUsedVertexRenderGroup(perFrameRenderGroup);
    ClearUsedRectInfoRenderGroup(perFrameRenderGroup);
}

void DoEditModeUI(GameMetadata *gameMetadata, RenderGroup *perFrameRenderGroup)
{
    const b32 skipFilter = true;
    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;
    f32 screenHeight = gameMetadata->screenResolution.v[1];

    f32 scaleFactor = 0.50f;
    f32 padding = 0.0f;
    f32 rectHeight = gameMetadata->fontBitmap.height / screenHeight * scaleFactor;

    v3 startingPosition = v3{ 0.0f, -1 + rectHeight * 0.5f, 0 };
    Rect *bottomUIBar =
        CreateRectangle(perFrameMemory, startingPosition, COLOR_BLACK - TRANSPARENCY(0.5f), 2, rectHeight);
    bottomUIBar->bitmapID = gameMetadata->whiteBitmap.bitmapID;
    bottomUIBar->bitmap = &gameMetadata->whiteBitmap;
    PushRenderGroupRectInfo(perFrameRenderGroup, bottomUIBar, skipFilter);

    padding = 0.02f;
    startingPosition = v3{ -1 + padding, -1 + rectHeight * 0.5f, 0 };
    PushStringRectToRenderGroup(
            perFrameRenderGroup, gameMetadata, perFrameMemory, startingPosition, scaleFactor, "Edit");

    if (gameMetadata->isCommandPrompt)
    {
        f32 scale = 0.50f;
        f32 rectHeight = gameMetadata->fontBitmap.height / screenHeight * scale;
        startingPosition =
            v3{ 0.0f, -0.35f + rectHeight * 0.5f, 0 };
        Rect *commandPromptBar =
            CreateRectangle(perFrameMemory, startingPosition, COLOR_BLACK - TRANSPARENCY(0.6f), 2, rectHeight);
        commandPromptBar->bitmapID = gameMetadata->whiteBitmap.bitmapID;
        commandPromptBar->bitmap = &gameMetadata->whiteBitmap;
        PushRenderGroupRectInfo(perFrameRenderGroup, commandPromptBar, skipFilter);

        startingPosition = v3{ -1 + padding, -0.35f + rectHeight * 0.5f, 0 };
        PushStringRectToRenderGroup(
                perFrameRenderGroup, gameMetadata, perFrameMemory, startingPosition, scaleFactor,
                gameMetadata->commandPrompt);

        /* TODO: Draw letters */
    }
}

void DrawUI(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        VulkanContext *vc,
        VulkanBuffers *vkBuffers,
        const f64 &MSPerFrame,
        const f64 &FPS,
        const f64 &MCPF)
{

    const b32 skipFilter = true;
    char buffer[256];
    f32 screenHeight = gameMetadata->screenResolution.v[1];
    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;

    sprintf_s(buffer, sizeof(char) * 150, "  %.02f ms/f    %.0ff/s    %.02fcycles/f  ", MSPerFrame, FPS, MCPF); // NOLINT

    f32 scaleFactor = 0.25f;

    /* This is in raw OpenGL coordinates */
    v3 startingPosition = v3{ -1, 1 - gameMetadata->fontBitmap.height / screenHeight * scaleFactor * 0.5f, 0 };

    PushStringRectToRenderGroup(
            perFrameRenderGroup, gameMetadata, perFrameMemory, startingPosition, scaleFactor, buffer);

    if (gameMetadata->isEditMode)
    {
        DoEditModeUI(gameMetadata, perFrameRenderGroup);
    }

    perFrameRenderGroup->rectCount = 0;
    ClearMemoryUsed(&perFrameRenderGroup->vertexMemory);

    /* TODO: Update UI texture */
    /* use texture of arrays or arrays of texture? */
    vkCmdNextSubpass(vc->drawCmd, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(vc->drawCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vc->pipeline2);
    vkCmdBindDescriptorSets(
            vc->drawCmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vc->pipelineLayout,
            0,
            1,
            &vc->secondDescSet,
            0,
            nullptr);
#if 0
    VulkanCopyImageFromHostToLocal(
            &vc-> device,
            stringBitmap.pitch,
            stringBitmap.height,
            vc->UITextures[0].image,
            vc->UITextures[0].mem,
            stringBitmap.size,
            stringBitmap.data);
#endif

    PushRectDynamicArrayToRenderGroupRectInfo(
            gameMetadata,
            perFrameRenderGroup,
            gameMetadata->rdaDebugUI,
            skipFilter);

    DrawRenderGroup(
            gameMetadata,
            perFrameRenderGroup,
            vc,
            vkBuffers);
}

void Render(GameMetadata *gameMetadata, VulkanContext *vc)
{
    RectDynamicArray *hitBoxes = gameMetadata->hitBoxes;
    RectDynamicArray *hurtBoxes = gameMetadata->hurtBoxes;
    RenderGroup *perFrameRenderGroup = gameMetadata->perFrameRenderGroup;

    f64 MSPerFrame = 0;
    f64 FPS = 0;
    f64 MCPF = 0;
    CalculateFrameStatistics(gameMetadata->gameTimestep, &MSPerFrame, &FPS, &MCPF);

    VulkanPrepareRender(vc);
    VulkanBeginRenderPass(vc);
    VulkanSetViewportAndScissor(vc);

    /* Should I differentiate between scene and ui rendergroups? */
    DrawScene(
        gameMetadata,
        perFrameRenderGroup,
        vc,
        &g_vkBuffers,
        g_camera,
        g_projection,
        hitBoxes,
        hurtBoxes);

    DrawUI(
        gameMetadata,
        perFrameRenderGroup,
        vc,
        &g_vkBuffers,
        MSPerFrame,
        FPS,
        MCPF);

    EndRender(gameMetadata, vc);
    RenderCleanup(gameMetadata, vc, &g_vkBuffers);
}
#endif
