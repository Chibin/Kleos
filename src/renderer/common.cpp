void Draw(
        GameMetadata *gameMetadata,
        RenderGroup *perFrameRenderGroup,
        VulkanContext *vc,
        VulkanBuffers *g_vkBuffers,
        Camera *g_camera,
        glm::mat4 *g_projection)
{
    if (gameMetadata->isVulkanActive)
    {
        if (perFrameRenderGroup->vertexMemory.used > 0)
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
        glm::mat4 correction = glm::mat4();
        correction[1][1] = -1;
        //correction[2][2] = 0.5;
        //correction[2][3] = 0.5;

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
