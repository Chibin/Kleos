/* Other vulkan definitions. Need a good way to organize this. */
void VulkanInitDepthBuffer(
        VulkanContext *vc,
        uint32 width,
        uint32 height,
        Depth *depth,
        VkDevice *device,
        VkCommandBuffer *setupCmd,
        VkCommandPool *cmdPool)
{
    VkResult err;
    const VkFormat depthFormat = VK_FORMAT_D16_UNORM;
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType =                    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext =                    NULL;
    imageInfo.flags =                    0;
    imageInfo.imageType =                VK_IMAGE_TYPE_2D;
    imageInfo.format =                   depthFormat;
    imageInfo.extent =                   VkExtent3D{width, height, 1};
    imageInfo.mipLevels =                1;
    imageInfo.arrayLayers =              1;
    imageInfo.samples =                  VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling =                   VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage =                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    depth->format = depthFormat;
    err = vkCreateImage(*device, &imageInfo, NULL, &depth->image);
    ASSERT(err == VK_SUCCESS);

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(*device, depth->image, &memReqs);

    VkMemoryAllocateInfo memAlloc = {
        /*.sType =*/            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        /*.pNext =*/            NULL,
        /*.allocationSize =*/   memReqs.size,
        /*.memoryTypeIndex =*/  0,
    };

    b32 pass = AvailableMemoryTypeFromProperties(
            &vc->memoryProperties,
            memReqs.memoryTypeBits,
            0 /*no requirements*/,
            &memAlloc.memoryTypeIndex);
    ASSERT(pass);

    err = vkAllocateMemory(*device, &memAlloc, NULL, &depth->mem);
    ASSERT(err == VK_SUCCESS);
    err = vkBindImageMemory(*device, depth->image, depth->mem, 0);
    ASSERT(err == VK_SUCCESS);

    VulkanAddPipelineBarrier(
            device,
            setupCmd,
            cmdPool,
            depth->image,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            (VkAccessFlagBits)0);

    VkImageViewCreateInfo viewInfo = {
        /*.sType =*/                VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        /*.pNext =*/                NULL,
        /*.flags =*/                0,
        /*.image =*/                depth->image,
        /*.viewType =*/             VK_IMAGE_VIEW_TYPE_2D,
        /*.format =*/               depth->format,
        /*.components = */          {},
        /*.subresourceRange =*/     { /*.aspectMask =*/     VK_IMAGE_ASPECT_DEPTH_BIT,
                                      /*.baseMipLevel =*/   0,
                                      /*.levelCount =*/     1,
                                      /*.baseArrayLayer =*/ 0,
                                      /*.layerCount =*/     1
                                    },
    };

    err = vkCreateImageView(*device, &viewInfo, NULL, &depth->view);
    ASSERT(err == VK_SUCCESS);

}

void VulkanPrepareTexture(VulkanContext *vc,
        VkPhysicalDevice *gpu,
        VkDevice *device,
        VkCommandBuffer *setupCmd,
        VkCommandPool *cmdPool,
        VkQueue *queue,
        VkPhysicalDeviceMemoryProperties *memoryProperties,
        bool useStagingBuffer,
        TextureObject *textures)
{
    const VkFormat texFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties props = {};

    vkGetPhysicalDeviceFormatProperties(*gpu, texFormat, &props);

    for (memory_index i = 0; i < DEMO_TEXTURE_COUNT; i++)
    {
        if ((props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) && !useStagingBuffer)
        {
            /* Device can texture using linear textures */
            ASSERT(textures[i].texWidth > 0);
            ASSERT(textures[i].texHeight > 0);
            ASSERT(textures[i].dataSize > 0);
            ASSERT(textures[i].data != NULL);

            VulkanSetTextureImage(
                    device,
                    setupCmd,
                    cmdPool,
                    memoryProperties,
                    &textures[i],
                    VK_IMAGE_TILING_LINEAR,
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    texFormat);

        }
        else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
        {

            VulkanUseStagingBufferToCopyLinearTextureToOptimized(
                    device,
                    setupCmd,
                    cmdPool,
                    queue,
                    memoryProperties,
                    &textures[i]
                    );

        }
        else
        {
            /* Can't support VK_FORMAT_R8G8B8A8_UNORM !? */
            ASSERT(!"No support for B8G8R8A8_UNORM as texture image format");
        }

        VulkanCreateImageSampler(
                device,
                texFormat,
                &textures[i]);
    }

}
