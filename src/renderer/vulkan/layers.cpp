VKAPI_ATTR VkBool32 VKAPI_CALL
BreakCallback(
        VkFlags msgFlags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject,
        size_t location,
        int32_t msgCode,
        const char *pLayerPrefix,
        const char *pMsg,
        void *pUserData)
{
    ASSERT(false);
    return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
DbgFunc(VkFlags msgFlags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject,
        size_t location,
        int32_t msgCode,
        const char *pLayerPrefix,
        const char *pMsg,
        void *pUserData)
{
    char *message = (char *)malloc(strlen(pMsg) + 100);

    ASSERT(message);

    auto validation_error = 1;

    if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        sprintf_s(message, strlen(pMsg) + 100, "ERROR: [%s] Code %d : \n%s", pLayerPrefix, msgCode, pMsg);
    }
    else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        sprintf_s(message, strlen(pMsg) + 100, "WARNING: [%s] Code %d : \n%s", pLayerPrefix, msgCode, pMsg);
    }
    else
    {
        return false;
    }

    printf("%s\n", message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "FOUND ERROR VALIDATION: DbgFunc", message, NULL);

    free(message);

    /*
     * false indicates that layer should not bail-out of an
     * API call that had validation failures. This may mean that the
     * app dies inside the driver due to invalid parameter(s).
     * That's what would happen without validation layers, so we'll
     * keep that behavior here.
     */
    return false;
}

/*
 * Return true if all layer names specified in checkNames
 * can be found in given layer properties.
 */
static VkBool32 VulkanCheckLayers(
        uint32_t check_count,
        char **checkNames,
        uint32_t layer_count,
        VkLayerProperties *layers)
{
    for (uint32_t i = 0; i < check_count; i++)
    {
        VkBool32 found = 0;
        for (uint32_t j = 0; j < layer_count; j++) {
            if (!strcmp(checkNames[i], layers[j].layerName))
            {
                found = 1;
                break;
            }
        }
        if (!found)
        {
            fprintf(stderr, "Cannot find layer: %s\n", checkNames[i]);
            return 0;
        }
    }

    return 1;
}

void VulkanSetInstanceLayer(
        u32 instanceLayerCount,
        char **standardInstanceLayers,
        char **altInstanceLayers,
        u32 standardLayerCount,
        u32 altLayerCount,
        char **o_deviceValidationLayers,
        u32 *o_deviceValidationLayerCount,
        u32 *o_enabledInstanceLayerCount)
{
    VkResult err;
	VkBool32 validationFound = 0;
	char **instanceValidationLayers = standardInstanceLayers;

	if (instanceLayerCount > 0)
    {
		VkLayerProperties *instanceLayers =
			(VkLayerProperties *)malloc(sizeof (VkLayerProperties) * instanceLayerCount);
		err = vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayers);
		ASSERT(!err);

		validationFound = VulkanCheckLayers(
				standardLayerCount,
				instanceValidationLayers,
                instanceLayerCount,
				instanceLayers);

		if (validationFound)
        {
			o_deviceValidationLayers[0] = "VK_LAYER_LUNARG_standard_validation";
			*o_enabledInstanceLayerCount = standardLayerCount;
			*o_deviceValidationLayerCount = 1;
		}
        else
        {
			// use alternative set of validation layers
			instanceValidationLayers = altInstanceLayers;
			validationFound = VulkanCheckLayers(
					altLayerCount,
					instanceValidationLayers,
                    instanceLayerCount,
					instanceLayers);

			for (uint32_t i = 0; i < altLayerCount; i++)
            {
				o_deviceValidationLayers[i] = instanceValidationLayers[i];
			}
			*o_enabledInstanceLayerCount = altLayerCount;
			*o_deviceValidationLayerCount = altLayerCount;
		}
		free(instanceLayers);
	}

	if (!validationFound)
    {
		ERROR_PRINT("vkEnumerateInstanceLayerProperties failed to find"
					"required validation layer.\n\n"
					"Please look at the Getting Started guide for additional "
					"information.\n"
					"vkCreateInstance Failure");
	}
}

void VulkanSetValidationLayer(
        VkPhysicalDevice *gpu,
        char **deviceValidationLayers,
        const u32 deviceValidationLayerCount,
        u32 *enabledValidationLayerCount)
{
    VkResult err;
    VkBool32 validationFound = 0;

    *enabledValidationLayerCount = 0;
    uint32_t deviceLayerCount = 0;
    err =
        vkEnumerateDeviceLayerProperties(*gpu, &deviceLayerCount, NULL);
    ASSERT(err == VK_SUCCESS);

    if (deviceLayerCount > 0)
    {
        VkLayerProperties *deviceLayers =
            (VkLayerProperties *)malloc(sizeof (VkLayerProperties) * deviceLayerCount);
        err = vkEnumerateDeviceLayerProperties(*gpu, &deviceLayerCount, deviceLayers);
        ASSERT(err == VK_SUCCESS);

        validationFound = VulkanCheckLayers(
                deviceValidationLayerCount,
                deviceValidationLayers,
                deviceLayerCount,
                deviceLayers);
        *enabledValidationLayerCount = deviceValidationLayerCount;

        free(deviceLayers);
    }

    if (!validationFound)
    {
        PAUSE_HERE("vkEnumerateDeviceLayerProperties failed to find "
                "a required validation layer.\n\n"
                "Please look at the Getting Started guide for additional "
                "information.\n"
                "vkCreateDevice Failure");
    }

}
