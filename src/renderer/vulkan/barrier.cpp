void VulkanSetSupportedStagesForImageLayout(
        VkImageLayout newImageLayout,
        VkImageMemoryBarrier *imageMemoryBarrier,
        VkPipelineStageFlags *o_srcStages,
        VkPipelineStageFlags *o_destStages)
{
        VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkPipelineStageFlags destStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        if (newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            // XXX: unsure when this needs to be set.
            srcStages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            destStages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }

        if (newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            srcStages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            destStages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        }

        if (imageMemoryBarrier->srcAccessMask & VK_ACCESS_HOST_WRITE_BIT)
        {
            srcStages = VK_PIPELINE_STAGE_HOST_BIT;
        }

        if (imageMemoryBarrier->dstAccessMask & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT)
        {
            destStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }

        if (newImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            destStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }

        *o_srcStages = srcStages;
        *o_destStages = destStages;
}

void VulkanAddPipelineBarrier(
        VkDevice *device,
        VkCommandBuffer *setupCmd,
        VkCommandPool *cmdPool,
        VkImage image,
        VkImageAspectFlags aspectMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkAccessFlagBits srcAccessMask)
{
        VkResult err;

        if (*setupCmd == VK_NULL_HANDLE)
        {
            VkCommandBufferAllocateInfo cmd = {};
            cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmd.pNext = nullptr;
            cmd.commandPool = *cmdPool;
            cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmd.commandBufferCount = 1;

            err = vkAllocateCommandBuffers(*device, &cmd, setupCmd);
            ASSERT(err == VK_SUCCESS);

            VkCommandBufferInheritanceInfo cmdBufHInfo = {};
            cmdBufHInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            cmdBufHInfo.pNext = nullptr;
            cmdBufHInfo.renderPass = VK_NULL_HANDLE;
            cmdBufHInfo.subpass = 0;
            cmdBufHInfo.framebuffer = VK_NULL_HANDLE;
            cmdBufHInfo.occlusionQueryEnable = VK_FALSE;
            cmdBufHInfo.queryFlags = 0;
            cmdBufHInfo.pipelineStatistics = 0;

            VkCommandBufferBeginInfo cmdBufInfo = {};
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBufInfo.pNext = nullptr;
            cmdBufInfo.flags = 0;
            cmdBufInfo.pInheritanceInfo = &cmdBufHInfo;

            err = vkBeginCommandBuffer(*setupCmd, &cmdBufInfo);
            ASSERT(err == VK_SUCCESS);
        }

        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.pNext = nullptr;
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = 0;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = {aspectMask, 0, 1, 0, 1};

        /* https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#synchronization-access-types-supported */
        switch(newImageLayout)
        {
            case (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL):
                /* Make sure anything that was copying from this image has completed */
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL):
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL):
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL):
                /* Make sure any Copy or CPU writes to image are flushed */
                imageMemoryBarrier.dstAccessMask =
                    VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                break;
        }

        VkImageMemoryBarrier *pImageMemoryBarrier = &imageMemoryBarrier;
        VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkPipelineStageFlags destStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VulkanSetSupportedStagesForImageLayout(
                newImageLayout,
                &imageMemoryBarrier,
                &srcStages,
                &destStages);

        vkCmdPipelineBarrier(*setupCmd, srcStages, destStages, 0, 0, nullptr, 0, nullptr, 1, pImageMemoryBarrier);
}
