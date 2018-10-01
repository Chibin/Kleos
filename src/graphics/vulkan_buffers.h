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
        Depth *depth,
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
    err = vc->fpGetPhysicalDeviceSurfacePresentModesKHR(*gpu, *surface, &presentModeCount, NULL);
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
        /*.pNext =*/                 NULL,
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
        /*.pQueueFamilyIndices =*/   NULL,
        /*.preTransform =*/          preTransform,
        /*.compositeAlpha =*/        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        /*.presentMode =*/           swapchainPresentMode,
        /*.clipped =*/               true,
        /*.oldSwapchain =*/          oldSwapchain,
    };

    err = vc->fpCreateSwapchainKHR(*device, &swapchainInfo, NULL, swapchain);
    ASSERT(err == VK_SUCCESS);

    // If we just re-created an existing swapchain, we should destroy the old
    // swapchain at this point.
    // Note: destroying the swapchain also cleans up all its associated
    // presentable images once the platform is done with them.
    if (oldSwapchain != VK_NULL_HANDLE)
    {
        vc->fpDestroySwapchainKHR(*device, oldSwapchain, NULL);
    }

    err = vc->fpGetSwapchainImagesKHR(*device, *swapchain, swapchainImageCount, NULL);
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
            /*.pNext =*/            NULL,
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
        err = vkCreateImageView(*device, &colorAttachmentView, NULL, &vc->buffers[i].view);
        ASSERT(!err);
    }

    *currentBuffer = 0;

    if (presentModes != NULL)
    {
        free(presentModes);
    }
}


