void VulkanSetInstance(
        VkInstance *inst,
        VkInstanceCreateInfo *instInfo)
{
    VkResult err;
	err = vkCreateInstance(instInfo, nullptr, inst);
	if (err == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		PAUSE_HERE("Cannot find a compatible Vulkan installable client driver "
				   "(ICD).\n\nPlease look at the Getting Started guide for "
				   "additional information.\n"
				   "vkCreateInstance Failure");
	}
	else if (err == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		PAUSE_HERE("Cannot find a specified extension library"
				   ".\nMake sure your layers path is set appropriately\n"
				   "vkCreateInstance Failure");
	}
	else if (err)
	{
		PAUSE_HERE("vkCreateInstance failed.\n\nDo you have a compatible Vulkan "
				"installable client driver (ICD) installed?\nPlease look at "
				"the Getting Started guide for additional information.\n"
				"vkCreateInstance Failure");
	}
}
