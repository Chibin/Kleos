void Draw(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        VulkanContext *vc,
        VulkanBuffers *g_vkBuffers,
        Camera *g_camera,
        glm::mat4 *g_projection)
{
    if (perFrameRenderGroup->vertexMemory.used == 0)
    {
        return;
    }

    if (gameMetadata->isVulkanActive)
    {
        ASSERT(perFrameRenderGroup->vertexMemory.used > 0);
        memset(&vc->vertices, 0, sizeof(vc->vertices));
        VulkanPrepareVertices(
                vc,
                &g_vkBuffers->bufs[g_vkBuffers->count],
                &g_vkBuffers->mems[g_vkBuffers->count],
                (void *)perFrameRenderGroup->vertexMemory.base,
                perFrameRenderGroup->vertexMemory.used);

        VulkanAddDrawCmd(
                vc,
                &g_vkBuffers->bufs[g_vkBuffers->count++],
                SafeCastToU32(perFrameRenderGroup->rectCount * 6));
    }

    if (gameMetadata->isOpenGLActive)
    {
        glBufferData(GL_ARRAY_BUFFER, perFrameRenderGroup->vertexMemory.used,
                perFrameRenderGroup->vertexMemory.base, GL_STATIC_DRAW);
        DrawRawRectangle(perFrameRenderGroup->rectCount);
    }
}

void UpdateUBOandPushConstants(
        GameMetadata *gameMetadata,
        VulkanContext *vc,
        Camera *g_camera,
        glm::mat4 *g_projection,
        GLuint *viewLoc,
        GLuint *projectionLoc)
{
    struct PushConstantMatrix
    {
        glm::mat4 view;
        glm::mat4 proj;
    } pushConstants;
    pushConstants = {};

    if (gameMetadata->isVulkanActive)
    {
        glm::mat4 correction = glm::mat4(1.0);
        correction[1][1] = -1;
        correction[2][2] = 0.5;
        correction[3][2] = 0.5;

        pushConstants.proj = correction * (*g_projection);
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

    if (gameMetadata->isOpenGLActive)
    {
        glUniformMatrix4fv(*viewLoc, 1, GL_FALSE, glm::value_ptr(g_camera->view));
        glUniformMatrix4fv(*projectionLoc, 1, GL_FALSE, glm::value_ptr(*g_projection));
    }
}

void UpdateSamplerImage(
        GameMetadata *gameMetadata,
        VulkanContext *vc,
        Bitmap *bitmap,
        TextureParam *textureParam,
        GLuint *textureID)
{
    if (gameMetadata->isOpenGLActive)
    {
        OpenGLUpdateTextureParameter(textureParam);
        OpenGLLoadBitmap(bitmap, *textureID);
    }

    if (gameMetadata->isVulkanActive)
    {
        /* Ideally, this should be done with a mapping of the bitmap to
         * descriptor layout
         */
        if (gameMetadata->playerRect->bitmap == bitmap)
        {
            vkCmdBindDescriptorSets(
                    vc->drawCmd,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    vc->pipelineLayout,
                    0,
                    1,
                    &vc->playerDescSet,
                    0,
                    NULL);
        }
        else if (&gameMetadata->whiteBitmap == bitmap)
        {
            /* TODO: add descriptor set for whitebitmap */
        }
        else if (bitmap->bitmapID != 0)
        {
            vkCmdBindDescriptorSets(
                    vc->drawCmd,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    vc->pipelineLayout,
                    0,
                    1,
                    &vc->secondDescSet,
                    0,
                    nullptr);
        }
        else
        {
            vkCmdBindDescriptorSets(
                    vc->drawCmd,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    vc->pipelineLayout,
                    0,
                    1,
                    &vc->descSet,
                    0,
                    NULL);
        }
    }
}

void EndRender(
        GameMetadata *gameMetadata,
        VulkanContext *vc)
{
    if (gameMetadata->isVulkanActive)
    {
        VulkanEndBufferCommands(vc);
        VulkanEndRender(vc);
    }

    if (gameMetadata->isOpenGLActive)
    {
        glBindVertexArray(0);
        OpenGLCheckErrors();

        OpenGLEndUseProgram();
    }
}

void RenderCleanup(
        GameMetadata *gameMetadata,
        VulkanContext *vc,
        VulkanBuffers *g_vkBuffers)
{
    if (gameMetadata->isVulkanActive)
    {
        /* clean up */
        vkFreeMemory(vc->device, vc->vertices.mem, nullptr);
        vkDestroyBuffer(vc->device, vc->vertices.buf, nullptr);

        for (memory_index i = 0; i < g_vkBuffers->count; i++)
        {
            vkDestroyBuffer(vc->device, g_vkBuffers->bufs[i], nullptr);
            vkFreeMemory(vc->device, g_vkBuffers->mems[i], nullptr);
        }
        g_vkBuffers->count = 0;
    }
}

void DrawRenderGroup(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        Rect **sortedRectInfo,
        VulkanContext *vc,
        VulkanBuffers *g_vkBuffers,
        Camera *g_camera,
        glm::mat4 *g_projection,
        GLuint *textureID)
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
                    g_vkBuffers,
                    g_camera,
                    g_projection);

            ClearUsedVertexRenderGroup(perFrameRenderGroup);

            if (bitmap != prevBitmap)
            {
                UpdateSamplerImage(
                        gameMetadata,
                        vc,
                        bitmap,
                        &textureParam,
                        textureID);
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
            g_vkBuffers,
            g_camera,
            g_projection);

    ClearUsedVertexRenderGroup(perFrameRenderGroup);
}

void DrawRenderGroup(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        VulkanContext *vc,
        VulkanBuffers *g_vkBuffers,
        Camera *g_camera,
        glm::mat4 *g_projection,
        GLuint *textureID)
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
                    g_vkBuffers,
                    g_camera,
                    g_projection);

            ClearUsedVertexRenderGroup(perFrameRenderGroup);

            if (bitmap != prevBitmap)
            {
                UpdateSamplerImage(
                        gameMetadata,
                        vc,
                        bitmap,
                        &textureParam,
                        textureID);
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
            g_vkBuffers,
            g_camera,
            g_projection);

    ClearUsedVertexRenderGroup(perFrameRenderGroup);
}
