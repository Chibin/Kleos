void VulkanSetPhysicalDevice(
        VkInstance *inst,
        VkPhysicalDevice *gpu)
{
    /* Make initial call to query gpuCount, then second call for gpu info*/
	u32 gpuCount;
    ASSERT(vkEnumeratePhysicalDevices(*inst, &gpuCount, NULL) == VK_SUCCESS);

	if (gpuCount > 0)
    {
		VkPhysicalDevice *pysicalDevices =
			(VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpuCount);
		ASSERT(vkEnumeratePhysicalDevices(*inst, &gpuCount, pysicalDevices) == VK_SUCCESS);

        /* TODO: Do something smarter here otherthan just grabbing the first physical device */
		*gpu = pysicalDevices[0];

		free(pysicalDevices);
	}
    else
    {
		ASSERT(!"vkEnumeratePhysicalDevices reported zero accessible devices."
                "\n\nDo you have a compatible Vulkan installable client"
                " driver (ICD) installed?\nPlease look at the Getting Started"
                " guide for additional information.\n"
                "vkEnumeratePhysicalDevices Failure");
	}

}

void VulkanInitQueueProperties(
        VkPhysicalDevice *gpu,
        u32 *o_queueCount,
        VkQueueFamilyProperties **queueProps)
{
    u32 queueCount = 0;
	// Query with NULL data to get count
	vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueCount, NULL);

	*queueProps = (VkQueueFamilyProperties *)malloc(queueCount * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(*gpu, &queueCount, *queueProps);
	ASSERT(queueCount >= 1);

    *o_queueCount = queueCount;
}
