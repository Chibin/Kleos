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
    imageInfo.pNext =                    nullptr;
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
    err = vkCreateImage(*device, &imageInfo, nullptr, &depth->image);
    ASSERT(err == VK_SUCCESS);

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(*device, depth->image, &memReqs);

    VkMemoryAllocateInfo memAlloc = {
        /*.sType =*/            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        /*.pNext =*/            nullptr,
        /*.allocationSize =*/   memReqs.size,
        /*.memoryTypeIndex =*/  0,
    };

    b32 pass = AvailableMemoryTypeFromProperties(
            &vc->memoryProperties,
            memReqs.memoryTypeBits,
            0 /*no requirements*/,
            &memAlloc.memoryTypeIndex);
    ASSERT(pass);

    err = vkAllocateMemory(*device, &memAlloc, nullptr, &depth->mem);
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
        /*.pNext =*/                nullptr,
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

    err = vkCreateImageView(*device, &viewInfo, nullptr, &depth->view);
    ASSERT(err == VK_SUCCESS);

}

