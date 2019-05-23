void Draw(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        VulkanContext *vc,
        VulkanBuffers *vkBuffers)
{
    if (perFrameRenderGroup->vertexMemory.used == 0)
    {
        return;
    }

    ASSERT(perFrameRenderGroup->vertexMemory.used > 0);
    memset(&vc->vertices, 0, sizeof(vc->vertices));
    VulkanPrepareVertices(
            vc,
            &vkBuffers->bufs[vkBuffers->count],
            &vkBuffers->mems[vkBuffers->count],
            (void *)perFrameRenderGroup->vertexMemory.base,
            perFrameRenderGroup->vertexMemory.used);

    VulkanAddDrawCmd(
            vc,
            &vkBuffers->bufs[vkBuffers->count++],
            SafeCastToU32(perFrameRenderGroup->rectCount * 6));

}

void UpdateUBOandPushConstants(
        GameMetadata *gameMetadata,
        VulkanContext *vc,
        Camera *g_camera,
        glm::mat4 *g_projection)
{
    struct PushConstantMatrix
    {
        glm::mat4 view;
        glm::mat4 proj;
    } pushConstants;
    pushConstants = {};

    pushConstants.proj = *g_projection;
    pushConstants.view = g_camera->view;
    vkCmdPushConstants(
            vc->drawCmd,
            vc->pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            sizeof(PushConstantMatrix),
            &pushConstants);

    //VulkanUpdateUniformBuffer(vc, &ubo);
}

void UpdateSamplerImage(
        GameMetadata *gameMetadata,
        VulkanContext *vc,
        Bitmap *bitmap,
        TextureParam *textureParam)
{
    Hash *hash = gameMetadata->hash;
    VkDescriptorSet *descSet = nullptr;
    HASH_GET_VALUE(HashKeyBitmapValueVkDescriptorSet, hash, bitmap, descSet);
    ASSERT(descSet != NULL);

    vkCmdBindDescriptorSets(
            vc->drawCmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vc->pipelineLayout,
            0,
            1,
            descSet,
            0,
            nullptr);
}

void EndRender(
        GameMetadata *gameMetadata,
        VulkanContext *vc)
{
    VulkanEndBufferCommands(vc);
    VulkanEndRender(vc);
}

void RenderCleanup(
        GameMetadata *gameMetadata,
        VulkanContext *vc,
        VulkanBuffers *vkBuffers)
{
    /* clean up */
    vkFreeMemory(vc->device, vc->vertices.mem, nullptr);
    vkDestroyBuffer(vc->device, vc->vertices.buf, nullptr);

    for (memory_index i = 0; i < vkBuffers->count; i++)
    {
        vkDestroyBuffer(vc->device, vkBuffers->bufs[i], nullptr);
        vkFreeMemory(vc->device, vkBuffers->mems[i], nullptr);
    }
    vkBuffers->count = 0;
}

void DrawRenderGroup(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        Rect **sortedRectInfo,
        VulkanContext *vc,
        VulkanBuffers *vkBuffers)
{
    Bitmap *bitmap = nullptr;
    Bitmap *prevBitmap = nullptr;
    TextureParam prevTextureParam = {};

    for (memory_index i = 0; i < perFrameRenderGroup->rectEntityCount; i++)
    {
        Rect *rect = (Rect *)sortedRectInfo[i];

        bitmap = rect->bitmap;
        ASSERT(bitmap != nullptr);

        TextureParam textureParam = bitmap->textureParam;

        if ((bitmap != prevBitmap) ||
            (textureParam != prevTextureParam))
        {

            Draw(
                    gameMetadata,
                    perFrameRenderGroup,
                    vc,
                    vkBuffers);

            ClearUsedVertexRenderGroup(perFrameRenderGroup);

            if (bitmap != prevBitmap)
            {
                UpdateSamplerImage(
                        gameMetadata,
                        vc,
                        bitmap,
                        &textureParam);
            }

            prevBitmap = bitmap;
        }

        PushRenderGroupRectVertex(perFrameRenderGroup, rect);
        prevTextureParam = textureParam;
    }

    Draw(
            gameMetadata,
            perFrameRenderGroup,
            vc,
            vkBuffers);

    ClearUsedVertexRenderGroup(perFrameRenderGroup);
}

void DrawRenderGroup(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        VulkanContext *vc,
        VulkanBuffers *vkBuffers)
{
    Bitmap *bitmap = nullptr;
    Bitmap *prevBitmap = nullptr;
    TextureParam prevTextureParam = {};

    for (memory_index i = 0; i < perFrameRenderGroup->rectEntityCount; i++)
    {
        Rect *rect = (Rect *)perFrameRenderGroup->rectMemory.base + i;

        bitmap = rect->bitmap;
        ASSERT(bitmap != nullptr);

        TextureParam textureParam = bitmap->textureParam;

        if ((bitmap != prevBitmap) ||
            (textureParam != prevTextureParam))
        {

            Draw(
                    gameMetadata,
                    perFrameRenderGroup,
                    vc,
                    vkBuffers);

            ClearUsedVertexRenderGroup(perFrameRenderGroup);

            if (bitmap != prevBitmap)
            {
                UpdateSamplerImage(
                        gameMetadata,
                        vc,
                        bitmap,
                        &textureParam);
            }

            prevBitmap = bitmap;
        }

        PushRenderGroupRectVertex(perFrameRenderGroup, rect);
        prevTextureParam = textureParam;
    }

    Draw(
            gameMetadata,
            perFrameRenderGroup,
            vc,
            vkBuffers);

    ClearUsedVertexRenderGroup(perFrameRenderGroup);
}
