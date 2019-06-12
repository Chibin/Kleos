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

    ASSERT(gameMetadata->rectManager->Traversable.rda.size > 0);

    RectDynamicArray *newWorldObjects = CreateRDAForNewWorldObjects(gameMetadata);

    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, &gameMetadata->rectManager->Traversable.rda, skipFilter);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, &gameMetadata->rectManager->NonTraversable.rda, skipFilter);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, newWorldObjects, skipFilter);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, hitBoxes, skipFilter);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, hurtBoxes, skipFilter);
    PushRectDynamicArrayToRenderGroupRectInfo(gameMetadata, perFrameRenderGroup, gameMetadata->rdaDebug, skipFilter);

    PushRenderGroupRectInfo(perFrameRenderGroup,
            HashGetValue(HashEntityRect, gameMetadata->hashEntityRect, gameMetadata->playerEntity), skipFilter);

    FOR_EACH_HASH_KEY_VAL_BEGIN(HashEntityNPC, hashKeyVal, gameMetadata->hashEntityNPC)
    {
        NPC *npc = hashKeyVal->val;
        Rect *minimalRect = CreateMinimalRectInfo(perFrameMemory, npc);
        UpdateNPCAnimation(npc, minimalRect);
        PushRenderGroupRectInfo(perFrameRenderGroup, minimalRect, skipFilter);
    }
    FOR_EACH_HASH_KEY_VAL_END();

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

void DrawUI(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroupUI,
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
            perFrameRenderGroupUI, gameMetadata, perFrameMemory, startingPosition, scaleFactor, buffer);

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
            perFrameRenderGroupUI,
            gameMetadata->rdaDebugUI,
            skipFilter);

    DrawRenderGroup(
            gameMetadata,
            perFrameRenderGroupUI,
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
        gameMetadata->camera,
        gameMetadata->projection,
        hitBoxes,
        hurtBoxes);

    DrawUI(
        gameMetadata,
        gameMetadata->perFrameRenderGroupUI,
        vc,
        &g_vkBuffers,
        MSPerFrame,
        FPS,
        MCPF);

    EndRender(gameMetadata, vc);
    RenderCleanup(gameMetadata, vc, &g_vkBuffers);
}
#endif
