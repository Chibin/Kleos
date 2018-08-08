void VulkanSetSurfaceExtensionNames(
        char **o_extensionNames,
        u32 *o_enabledExtensionCount,
        b32 validate)
{
    VkResult err;
	VkBool32 surfaceExtFound = 0;
	VkBool32 platformSurfaceExtFound = 0;
    u32 instanceExtensionCount = 0;
    u32 extensionCount = 0;

	err = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
	ASSERT(err == VK_SUCCESS);

	if (instanceExtensionCount > 0)
    {
		VkExtensionProperties *instanceExtensions =
			(VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
		err = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, instanceExtensions);
		ASSERT(err == VK_SUCCESS);

		for (uint32_t i = 0; i < instanceExtensionCount; i++)
        {
			printf("%s\n", instanceExtensions[i].extensionName);
			if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instanceExtensions[i].extensionName))
            {
				surfaceExtFound = 1;
				o_extensionNames[extensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
			}

			if (!strcmp(PLATFORM_SURFACE_EXTENSION_NAME, instanceExtensions[i].extensionName))
            {
				platformSurfaceExtFound = 1;
				o_extensionNames[extensionCount++] = PLATFORM_SURFACE_EXTENSION_NAME;
			}

			if (!strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, instanceExtensions[i].extensionName))
            {
				if (validate)
                {
					o_extensionNames[extensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
				}
			}

			ASSERT(extensionCount < 64);
		}

		free(instanceExtensions);
	}

    *o_enabledExtensionCount = extensionCount;

	if (!surfaceExtFound)
    {
		PAUSE_HERE("vkEnumerateInstanceExtensionProperties failed to find "
                   "the " VK_KHR_SURFACE_EXTENSION_NAME
                   " extension.\n\nDo you have a compatible "
                   "Vulkan installable client driver (ICD) installed?\nPlease "
                   "look at the Getting Started guide for additional "
                   "information.\n"
                   "vkCreateInstance Failure");
	}

	if (!platformSurfaceExtFound)
    {
		PAUSE_HERE("vkEnumerateInstanceExtensionProperties failed to find "
                   "the " PLATFORM_SURFACE_EXTENSION_NAME
                   " extension.\n\nDo you have a compatible "
                   "Vulkan installable client driver (ICD) installed?\nPlease "
                   "look at the Getting Started guide for additional "
                   "information.\n"
                   "vkCreateInstance Failure");
	}

}

void VulkanSetSwapchainExtensionName(
        VkPhysicalDevice *gpu,
        char **o_extensionNames,
        u32 *o_enabledExtensionCount)
{
    VkResult err;
	u32 deviceExtensionCount = 0;
	VkBool32 swapchainExtFound = 0;
	u32 extensionCount = 0;

	err = vkEnumerateDeviceExtensionProperties(*gpu, NULL, &deviceExtensionCount, NULL);
	ASSERT(err == VK_SUCCESS);

	if (deviceExtensionCount > 0)
	{
		VkExtensionProperties *deviceExtensions =
			(VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);
		err = vkEnumerateDeviceExtensionProperties(*gpu, NULL, &deviceExtensionCount, deviceExtensions);
		ASSERT(err == VK_SUCCESS);

		for (memory_index i = 0; i < deviceExtensionCount; i++)
        {
            /* strcmp returns 0 if they are both equal */
			if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, deviceExtensions[i].extensionName))
			{
				swapchainExtFound = 1;
				o_extensionNames[extensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			}

			ASSERT(extensionCount < 64);
		}

		free(deviceExtensions);
	}

    *o_enabledExtensionCount = extensionCount;

	if (!swapchainExtFound)
	{
		PAUSE_HERE("vkEnumerateDeviceExtensionProperties failed to find "
				"the " VK_KHR_SWAPCHAIN_EXTENSION_NAME
				" extension.\n\nDo you have a compatible "
				"Vulkan installable client driver (ICD) installed?\nPlease "
				"look at the Getting Started guide for additional "
				"information.\n"
				"vkCreateInstance Failure");
	}
}

void VulkanGetDebugEXTFunctions(
        VkInstance *inst,
        PFN_vkCreateDebugReportCallbackEXT &fpCreateDebugReportCallbackEXT,
        PFN_vkDestroyDebugReportCallbackEXT &fpDestroyDebugReportCallbackEXT,
        PFN_vkDebugReportMessageEXT &fpDebugReportMessageEXT,
        b32 useBreak,
        VkDebugReportCallbackEXT *msgCallback)
{
    VkResult err;

    GET_INSTANCE_PROC_ADDR(*inst, CreateDebugReportCallbackEXT);
    GET_INSTANCE_PROC_ADDR(*inst, DestroyDebugReportCallbackEXT);
    GET_INSTANCE_PROC_ADDR(*inst, DebugReportMessageEXT);

    VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
    dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    dbgCreateInfo.pfnCallback = useBreak ? BreakCallback : DbgFunc;
    dbgCreateInfo.pUserData = NULL;
    dbgCreateInfo.pNext = NULL;

    err = fpCreateDebugReportCallbackEXT(*inst, &dbgCreateInfo, NULL, msgCallback);
    switch (err)
    {
        case VK_SUCCESS:
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            PAUSE_HERE("CreateDebugReportCallback: out of host memory\n"
                       "CreateDebugReportCallback Failure");
            break;
        default:
            PAUSE_HERE("CreateDebugReportCallback: unknown failure\n"
                       "CreateDebugReportCallback Failure");
            break;
    }

}

