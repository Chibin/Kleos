void VulkanInitDevice(VulkanContext *vc,
        VkDevice *device,
        VkPhysicalDevice *gpu,
        const u32 graphicsQueueNodeIndex,
        const u32 enabledLayerCount,
        const u32 enabledExtensionCount,
        char **extensionNames,
        bool validate,
        char **deviceValidationLayers)
{
    VkResult err;
    float queuePriorities[1] = {0.0};
    const VkDeviceQueueCreateInfo queueInfo = {
        /*.sType =*/ 			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        /*.pNext =*/ 			NULL,
		/*.flags =*/			false,
        /*.queueFamilyIndex =*/ graphicsQueueNodeIndex,
        /*.queueCount =*/ 		1,
        /*.pQueuePriorities =*/ queuePriorities};

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.shaderClipDistance = VK_TRUE;

    VkDeviceCreateInfo deviceInfo = {
        /*.sType =*/ 			       VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        /*.pNext =*/ 			       NULL,
		/*.flags =*/				   false,
        /*.queueCreateInfoCount =*/    1,
        /*.pQueueCreateInfos =*/  	   &queueInfo,
        /*.enabledLayerCount =*/  	   enabledLayerCount,
        /*.ppEnabledLayerNames =*/     (const char *const *)((validate) ? deviceValidationLayers : NULL),
        /*.enabledExtensionCount =*/   enabledExtensionCount,
        /*.ppEnabledExtensionNames =*/ (const char *const *)extensionNames,
        /*.pEnabledFeatures =*/ 	   &deviceFeatures,
    };

    err = vkCreateDevice(*gpu, &deviceInfo, NULL, device);
	ASSERT(err == VK_SUCCESS);
}

