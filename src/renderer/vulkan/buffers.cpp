VkPresentModeKHR VulkanGetPresentMode(
        VkPresentModeKHR *presentModes,
        u32 presentModeCount,
        b32 vsync)
{
    /* The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
     * This mode waits for the vertical blank ("v-sync")
     */
     VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    /* If v-sync is not requested, try to find a mailbox mode.
     * It's the lowest latency non-tearing present mode available
     */
    if (!vsync)
    {
        for (memory_index i = 0; i < presentModeCount; i++)
        {
            if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) &&
                    (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
            {
                swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    return swapchainPresentMode;
}

void VulkanPrepareSwapchain(VulkanContext *vc,
        VkCommandBuffer *setupCmd,
        VkCommandPool *cmdPool,
        VkPhysicalDevice *gpu,
        VkSurfaceKHR *surface,
        VkFormat *format,
        VkSwapchainKHR *swapchain,
        uint32 *width, 
        uint32 *height,
        VkDevice *device,
        VkColorSpaceKHR colorSpace,
        u32 *swapchainImageCount,
        u32 *currentBuffer)
{
    /* Swapchain is a list/chain of image buffers */

    VkResult err;
    VkSwapchainKHR oldSwapchain = *swapchain;

    /* Check the surface capabilities and formats */
    VkSurfaceCapabilitiesKHR surfCapabilities;
    err = vc->fpGetPhysicalDeviceSurfaceCapabilitiesKHR(*gpu, *surface, &surfCapabilities);
    ASSERT(err == VK_SUCCESS);

    u32 presentModeCount;
    err = vc->fpGetPhysicalDeviceSurfacePresentModesKHR(*gpu, *surface, &presentModeCount, nullptr);
    ASSERT(err == VK_SUCCESS);

    VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    ASSERT(presentModes);

    err = vc->fpGetPhysicalDeviceSurfacePresentModesKHR(*gpu, *surface, &presentModeCount, presentModes);
    ASSERT(err == VK_SUCCESS);

    VkExtent2D swapchainExtent;
    // width and height are either both -1, or both not -1.
    if (surfCapabilities.currentExtent.width == (uint32_t)-1)
    {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
        swapchainExtent.width = *width;
        swapchainExtent.height = *height;
    }
    else
    {
        printf("width %d height %d\n", surfCapabilities.currentExtent.width, surfCapabilities.currentExtent.height);
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfCapabilities.currentExtent;
        *width = surfCapabilities.currentExtent.width;
        *height = surfCapabilities.currentExtent.height;
    }

    /* Determine the number of VkImage's to use in the swap chain (we desire to
     * own only 1 image at a time, besides the images being displayed and
     * queued for display)
     */
    uint32_t desiredNumberOfSwapchainImages = surfCapabilities.minImageCount + 1;
    if ((surfCapabilities.maxImageCount > 0) &&
            (desiredNumberOfSwapchainImages > surfCapabilities.maxImageCount))
    {
        /* A value of 0 for maxImageCount means that there is no limit besides
         * memory requirements
         */
        desiredNumberOfSwapchainImages = surfCapabilities.maxImageCount;
    }

    VkSurfaceTransformFlagBitsKHR preTransform;
    preTransform =
        (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ?
        VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR :
        surfCapabilities.currentTransform;

    b32 vsync = false;
    /* Select a present mode for the swapchain */
    VkPresentModeKHR swapchainPresentMode =
        VulkanGetPresentMode(
                presentModes,
                presentModeCount,
                vsync);

    const VkSwapchainCreateInfoKHR swapchainInfo = {
        /*.sType =*/                 VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        /*.pNext =*/                 nullptr,
        /*.flags =*/                 false,
        /*.surface =*/               *surface,
        /*.minImageCount =*/         desiredNumberOfSwapchainImages,
        /*.imageFormat =*/           *format,
        /*.imageColorSpace =*/       colorSpace,
        /*.imageExtent =*/           {/*.width =*/  swapchainExtent.width,
                                      /*.height =*/ swapchainExtent.height,
                                     },
        /*.imageArrayLayers =*/      1,
        /*.imageUsage =*/            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        /*.imageSharingMode =*/      VK_SHARING_MODE_EXCLUSIVE,
        /*.queueFamilyIndexCount =*/ 0,
        /*.pQueueFamilyIndices =*/   nullptr,
        /*.preTransform =*/          preTransform,
        /*.compositeAlpha =*/        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        /*.presentMode =*/           swapchainPresentMode,
        /*.clipped =*/               true,
        /*.oldSwapchain =*/          oldSwapchain,
    };

    err = vc->fpCreateSwapchainKHR(*device, &swapchainInfo, nullptr, swapchain);
    ASSERT(err == VK_SUCCESS);

    // If we just re-created an existing swapchain, we should destroy the old
    // swapchain at this point.
    // Note: destroying the swapchain also cleans up all its associated
    // presentable images once the platform is done with them.
    if (oldSwapchain != VK_NULL_HANDLE)
    {
        vc->fpDestroySwapchainKHR(*device, oldSwapchain, nullptr);
    }

    err = vc->fpGetSwapchainImagesKHR(*device, *swapchain, swapchainImageCount, nullptr);
    ASSERT(err == VK_SUCCESS);

    VkImage *swapchainImages = (VkImage *)malloc(*swapchainImageCount * sizeof(VkImage));
    ASSERT(swapchainImages);
    err = vc->fpGetSwapchainImagesKHR(*device, *swapchain, swapchainImageCount, swapchainImages);
    ASSERT(err == VK_SUCCESS);

    vc->buffers = (SwapchainBuffers *)malloc(sizeof(SwapchainBuffers) * (*swapchainImageCount));
    ASSERT(vc->buffers);

    for (memory_index i = 0; i < *swapchainImageCount; i++)
    {

        vc->buffers[i].image = swapchainImages[i];

        VkImageViewCreateInfo colorAttachmentView = {
            /*.sType =*/            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            /*.pNext =*/            nullptr,
            /*.flags =*/            0,
            /* XXX: this is added by me. not sure if it will work */
            /*.image =*/            swapchainImages[i],
            /*.viewType =*/         VK_IMAGE_VIEW_TYPE_2D,
            /*.format =*/           *format,
            /*.components =*/       {/*.r =*/ VK_COMPONENT_SWIZZLE_R,
                                     /*.g =*/ VK_COMPONENT_SWIZZLE_G,
                                     /*.b =*/ VK_COMPONENT_SWIZZLE_B,
                                     /*.a =*/ VK_COMPONENT_SWIZZLE_A,
                                    },
            /*.subresourceRange =*/ {/*.aspectMask =*/     VK_IMAGE_ASPECT_COLOR_BIT,
                                     /*.baseMipLevel =*/   0,
                                     /*.levelCount =*/     1,
                                     /*.baseArrayLayer =*/ 0,
                                     /*.layerCount =*/     1
                                    },
        };

        /* Render loop will expect image to have been used before and in
         * VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
         * layout and will change to COLOR_ATTACHMENT_OPTIMAL, so init the image
         * to that state
         */
        VulkanAddPipelineBarrier(
                device,
                setupCmd,
                cmdPool,
                vc->buffers[i].image,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                (VkAccessFlagBits)0);

        colorAttachmentView.image = vc->buffers[i].image;
        err = vkCreateImageView(*device, &colorAttachmentView, nullptr, &vc->buffers[i].view);
        ASSERT(!err);
    }

    *currentBuffer = 0;

    if (presentModes != nullptr)
    {
        free(presentModes);
    }
}

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


