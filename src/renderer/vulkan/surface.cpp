#define PLATFORM_ASSIGN_SURFACEINFO \
    createInfo.hinstance = GetModuleHandle(NULL); \
    createInfo.hwnd = info.info.win.window

void VulkanCheckQueueFamilyCompatability(
        const u32 graphicsQueueNodeIndex,
        const u32 presentQueueNodeIndex);

void VulkanGetSupportedQueueFamily(VulkanContext *vc,
        VkInstance *inst,
        VkPhysicalDevice *gpu,
        VkSurfaceKHR *surface,
        SDL_Window **window,
        VkQueueFamilyProperties *queueProps,
        u32 *graphicsQueueNodeIndex,
        u32 *presentQueueNodeIndex)
{
    /* Find physical device surface that will let us present and draw 
     * Able to use VK_QUEUE_GRAPHICS_BIT and supports "presenting"
     */
    VkResult err;
    u32 queueCount = 0;

	// Create a WSI surface for the window
	PlatformSurfaceCreateInfo createInfo;
	createInfo.sType = PLATFORM_SURFACE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.flags = 0;

	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(*window, &info);

	PLATFORM_ASSIGN_SURFACEINFO;

	err = PlatformCreateSurface(*inst, &createInfo, NULL, surface);

	// Query with NULL data to get count
	vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueCount, NULL);
    //
	// Iterate over each queue to learn whether it supports presenting
	VkBool32 *supportsPresent = (VkBool32 *)malloc(queueCount * sizeof(VkBool32));
	for (memory_index i = 0; i < queueCount; i++)
	{
        /* TODO: safecast */
		vc->fpGetPhysicalDeviceSurfaceSupportKHR(*gpu, (u32)i, *surface, &supportsPresent[i]);
	}

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	for (memory_index i = 0; i < queueCount; i++)
	{
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (*graphicsQueueNodeIndex == UINT32_MAX)
			{
                /* TODO: safecast */
				*graphicsQueueNodeIndex = (u32)i;
			}

			if (supportsPresent[i] == VK_TRUE)
			{
				*graphicsQueueNodeIndex = (u32)i;
				*presentQueueNodeIndex = (u32)i;
				break;
			}
		}
	}

	if (*presentQueueNodeIndex == UINT32_MAX)
    {
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (memory_index i = 0; i < queueCount; ++i)
		{
			if (supportsPresent[i] == VK_TRUE) {
				*presentQueueNodeIndex = (u32)i;
				break;
			}
		}
	}

	free(supportsPresent);

    VulkanCheckQueueFamilyCompatability(*graphicsQueueNodeIndex, *presentQueueNodeIndex);
}

void VulkanCheckQueueFamilyCompatability(
        const u32 graphicsQueueNodeIndex,
        const u32 presentQueueNodeIndex)
{
	// Generate error if could not find both a graphics and a present queue
	if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
	{
		PAUSE_HERE("Could not find a graphics and a present queue\n"
				"Swapchain Initialization Failure");
	}

	// TODO: Add support for separate queues, including presentation,
	//       synchronization, and appropriate tracking for QueueSubmit.
	// NOTE: While it is possible for an application to use a separate graphics
	//       and a present queues, this program assumes it is only using
	//       one
	if (graphicsQueueNodeIndex != presentQueueNodeIndex)
    {
		PAUSE_HERE("Could not find a common graphics and a present queue\n"
				"Swapchain Initialization Failure");
	}
}
