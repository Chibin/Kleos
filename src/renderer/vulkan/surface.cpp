#ifdef WIN32
#define PLATFORM_ASSIGN_SURFACEINFO \
    createInfo.hinstance = GetModuleHandle(NULL); \
    createInfo.hwnd = info.info.win.window
#else
#define PLATFORM_ASSIGN_SURFACEINFO \
        createInfo.connection = xconn; \
    createInfo.window = xcb_window;
#include <dlfcn.h>
#endif

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
    createInfo.pNext = nullptr;
    createInfo.flags = 0;

    struct SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
#ifdef WIN32
    SDL_GetWindowWMInfo(*window, &info);
#else
    SDL_GetWindowID(*window);

    xcb_connection_t *xconn = xcb_connect(NULL, NULL);
    xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(xconn)).data;
    xcb_window_t xcb_window = xcb_generate_id(xconn);
    xcb_create_window(xconn, XCB_COPY_FROM_PARENT, xcb_window, screen->root, 0, 0,
            SCREEN_WIDTH, SCREEN_HEIGHT, 10, XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual,
            0, NULL);
    xcb_map_window(xconn, xcb_window);
    xcb_flush(xconn);
#endif

    PLATFORM_ASSIGN_SURFACEINFO;

    err = PlatformCreateSurface(*inst, &createInfo, nullptr, surface);

    // Query with nullptr data to get count
    vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueCount, nullptr);
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
