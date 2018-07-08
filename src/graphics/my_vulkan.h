#ifndef __MY_VULKAN__
#define __MY_VULKAN__

#include "./vulkan.h"
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                               \
    {                                                                          \
        fp##entrypoint =                                                 	   \
            (PFN_vk##entrypoint)vkGetInstanceProcAddr(inst, "vk" #entrypoint); \
        if (fp##entrypoint == NULL) {                                          \
            PAUSE_HERE("vkGetInstanceProcAddr failed to find vk" #entrypoint   \
                     "vkGetInstanceProcAddr Failure");                         \
        }                                                                      \
    }

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                                  \
    {                                                                          \
        fp##entrypoint =                                                 	   \
            (PFN_vk##entrypoint)vkGetDeviceProcAddr(dev, "vk" #entrypoint);    \
        if (fp##entrypoint == NULL) {                                    	   \
            PAUSE_HERE("vkGetDeviceProcAddr failed to find vk" #entrypoint     \
                     "vkGetDeviceProcAddr Failure");                           \
        }                                                                      \
	}

VKAPI_ATTR VkBool32 VKAPI_CALL
BreakCallback(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType,
        uint64_t srcObject, size_t location, int32_t msgCode,
        const char *pLayerPrefix, const char *pMsg,
        void *pUserData)
{
    ASSERT(false);
    return false;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
dbgFunc(VkFlags msgFlags,
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
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "FOUND ERROR VALIDATION: dbgFunc", message, NULL);

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
 * Return 1 (true) if all layer names specified in checkNames
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

static bool AvailableMemoryTypeFromProperties(
        VkPhysicalDeviceMemoryProperties *memoryProperties,
        uint32_t typeBits,
        VkFlags requirements_mask,
        uint32_t *typeIndex)
{
    for (uint32_t i = 0; i < 32; i++)
    {
        if ((typeBits & 1) == 1)
        {
            // Type is available, does it match user properties?
            if ((memoryProperties->memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
            {
                *typeIndex = i;
                return true;
                break;
            }
        }
        typeBits >>= 1;
    }
    return false;
}

static VkShaderModule
VulkanPrepareShaderModule(VkDevice *device, const void *code, size_t size)
{
    VkShaderModuleCreateInfo moduleCreateInfo;
    VkShaderModule module;
    VkResult err;

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;

    moduleCreateInfo.codeSize = size;
    moduleCreateInfo.pCode = (const uint32_t *)code;
    moduleCreateInfo.flags = 0;
    err = vkCreateShaderModule(*device, &moduleCreateInfo, NULL, &module);
    ASSERT(!err);

    return module;
}

char *ReadSPV(const char *filename, size_t *psize)
{
    long int size;
    void *shaderCode;
    size_t retVal;

    FILE *fp = NULL;
    errno_t err = fopen_s(&fp, filename, "rb");
    if (err != 0)
    {
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);

    fseek(fp, 0L, SEEK_SET);

    shaderCode = malloc(size);
    retVal = fread(shaderCode, size, 1, fp);
    if (!retVal)
    {
        return NULL;
    }

    *psize = size;

    fclose(fp);
    return (char *)shaderCode;
}

static VkShaderModule VulkanPrepareVertexShader(
        VkDevice *device,
        VkShaderModule *vertShaderModule)
{
    void *vertShaderCode;
    size_t size;

    vertShaderCode = ReadSPV("./materials/programs/vulkan/test-vert.spv", &size);
    ASSERT(vertShaderCode != NULL);

    *vertShaderModule = VulkanPrepareShaderModule(device, vertShaderCode, size);

    free(vertShaderCode);

    return *vertShaderModule;
}

static VkShaderModule VulkanPrepareFragmentShader(
        VkDevice *device,
        VkShaderModule *fragShaderModule)
{
    void *fragShaderCode;
    size_t size;

    fragShaderCode = ReadSPV("./materials/programs/vulkan/test-frag.spv", &size);

    *fragShaderModule = VulkanPrepareShaderModule(device, fragShaderCode, size);

    free(fragShaderCode);

    return *fragShaderModule;
}

void VulkanSetImageLayout(
        VkDevice *device,
        VkCommandBuffer *setupCmd,
        VkCommandPool *cmdPool,
        VkImage image,
        VkImageAspectFlags aspectMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkAccessFlagBits srcAccessMask)
{
        VkResult err;

        if (*setupCmd == VK_NULL_HANDLE)
        {
            VkCommandBufferAllocateInfo cmd = {};
            cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmd.pNext = NULL;
            cmd.commandPool = *cmdPool;
            cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmd.commandBufferCount = 1;

            err = vkAllocateCommandBuffers(*device, &cmd, setupCmd);
            ASSERT(err == VK_SUCCESS);

            VkCommandBufferInheritanceInfo cmdBufHInfo = {};
            cmdBufHInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
            cmdBufHInfo.pNext = NULL;
            cmdBufHInfo.renderPass = VK_NULL_HANDLE;
            cmdBufHInfo.subpass = 0;
            cmdBufHInfo.framebuffer = VK_NULL_HANDLE;
            cmdBufHInfo.occlusionQueryEnable = VK_FALSE;
            cmdBufHInfo.queryFlags = 0;
            cmdBufHInfo.pipelineStatistics = 0;

            VkCommandBufferBeginInfo cmdBufInfo = {};
            cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBufInfo.pNext = NULL;
            cmdBufInfo.flags = 0;
            cmdBufInfo.pInheritanceInfo = &cmdBufHInfo;

            err = vkBeginCommandBuffer(*setupCmd, &cmdBufInfo);
            ASSERT(err == VK_SUCCESS);
        }

        VkImageMemoryBarrier imageMemoryBarrier = {};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.pNext = NULL;
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = 0;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = {aspectMask, 0, 1, 0, 1};

        /* https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#synchronization-access-types-supported */
        switch(newImageLayout)
        {
            case (VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL):
                /* Make sure anything that was copying from this image has completed */
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                break;

            case(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL):
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                break;

            case(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL):
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                break;

            case (VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL):
                /* Make sure any Copy or CPU writes to image are flushed */
                imageMemoryBarrier.dstAccessMask =
                    VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
                break;
        }

        VkImageMemoryBarrier *pmemoryBarrier = &imageMemoryBarrier;

        VkPipelineStageFlags srcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        VkPipelineStageFlags destStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        if (newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            // XXX: unsure when this needs to be set.
            srcStages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            destStages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }

        if (newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            srcStages =
                VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            destStages = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
        }

        if (srcAccessMask & VK_ACCESS_HOST_WRITE_BIT)
        {
            srcStages = VK_PIPELINE_STAGE_HOST_BIT;
        }

        if (imageMemoryBarrier.dstAccessMask & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT)
        {
            destStages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }

        if (newImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            destStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }

        vkCmdPipelineBarrier(*setupCmd, srcStages, destStages, 0, 0, NULL, 0, NULL, 1, pmemoryBarrier);
}

static void VulkanPrepareTextureImage(
        VkDevice *device,
        VkCommandBuffer *setupCmd,
        VkCommandPool *cmdPool,
        VkPhysicalDeviceMemoryProperties *memoryProperties,
        const uint32_t *texColors,
        struct TextureObject *texObj,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkFlags requiredProps)
{
            const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
            const int32_t texWidth = 2;
            const int32_t texHeight = 2;
            VkResult err;
            bool pass;

            texObj->texWidth = texWidth;
            texObj->texHeight = texHeight;

            VkImageCreateInfo image_create_info = {};
            image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            image_create_info.pNext = NULL;
            image_create_info.imageType = VK_IMAGE_TYPE_2D;
            image_create_info.format = tex_format;
            image_create_info.extent = {texWidth, texHeight, 1};
            image_create_info.mipLevels = 1;
            image_create_info.arrayLayers = 1;
            image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
            image_create_info.tiling = tiling;
            image_create_info.usage = usage;
            image_create_info.flags = 0;
            image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

            VkMemoryAllocateInfo memAlloc = {};
            memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            memAlloc.pNext = NULL;
            memAlloc.allocationSize = 0;
            memAlloc.memoryTypeIndex = 0;

            VkMemoryRequirements memReqs;

            err = vkCreateImage(*device, &image_create_info, NULL, &texObj->image);
            ASSERT(!err);

            vkGetImageMemoryRequirements(*device, texObj->image, &memReqs);

            memAlloc.allocationSize = memReqs.size;

            pass = AvailableMemoryTypeFromProperties(memoryProperties,
                                                     memReqs.memoryTypeBits,
                                                     requiredProps,
                                                     &memAlloc.memoryTypeIndex);
            ASSERT(pass);

            /* allocate memory */
            err = vkAllocateMemory(*device, &memAlloc, NULL, &texObj->mem);
            ASSERT(!err);

            /* bind memory */
            err = vkBindImageMemory(*device, texObj->image, texObj->mem, 0);
            ASSERT(!err);

            if (requiredProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
                const VkImageSubresource subres = {
                    /*.aspectMask =*/   VK_IMAGE_ASPECT_COLOR_BIT,
                    /*.mipLevel =*/     0,
                    /*.arrayLayer =*/   0,
                };

                VkSubresourceLayout layout;
                void *data;
                int32_t x, y;

                vkGetImageSubresourceLayout(*device, texObj->image, &subres, &layout);

                err = vkMapMemory(*device, texObj->mem, 0, memAlloc.allocationSize, 0, &data);
                ASSERT(!err);

                for (y = 0; y < texHeight; y++)
                {
                    uint32_t *row = (uint32_t *)((char *)data + layout.rowPitch * y);
                    for (x = 0; x < texWidth; x++)
                    {
                        row[x] = texColors[(x & 1) ^ (y & 1)];
                    }
                }

                vkUnmapMemory(*device, texObj->mem);
            }

            texObj->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            // XXX: tweaked the masks compatability
            VulkanSetImageLayout(
                    device,
                    setupCmd,
                    cmdPool,
                    texObj->image,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_PREINITIALIZED,
                    texObj->imageLayout,
                    VK_ACCESS_HOST_WRITE_BIT);
            /* setting the image layout does not reference the actual memory so no need
             * to add a mem ref
             */
}

void VulkanFlushInit(
        VkDevice *device,
        VkCommandBuffer *setupCmd,
        VkCommandPool *cmdPool,
        VkQueue *queue)
{
    VkResult err;

    if (*setupCmd == VK_NULL_HANDLE)
        return;

    err = vkEndCommandBuffer(*setupCmd);
    ASSERT(!err);

    const VkCommandBuffer cmdBufs[] = {*setupCmd};
    VkFence nullFence = {VK_NULL_HANDLE};
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = NULL;
    submitInfo.pWaitDstStageMask = NULL;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = cmdBufs;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = NULL;

    err = vkQueueSubmit(*queue, 1, &submitInfo, nullFence);
    ASSERT(!err);

    err = vkQueueWaitIdle(*queue);
    ASSERT(!err);

    vkFreeCommandBuffers(*device, *cmdPool, 1, cmdBufs);
    *setupCmd = VK_NULL_HANDLE;
}

void VulkanDestroyTextureImage(
        VkDevice *device,
        struct TextureObject *texObj)
{
        /* clean up staging resources */
        vkDestroyImage(*device, texObj->image, NULL);
        vkFreeMemory(*device, texObj->mem, NULL);
}

static void VulkanBuildDrawCommand(struct VulkanContext *vc)
{
    const VkCommandBufferInheritanceInfo cmdBufHInfo = {
        /*.sType =*/ VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        /*.pNext =*/ NULL,
        /*.renderPass =*/ VK_NULL_HANDLE,
        /*.subpass =*/ 0,
        /*.framebuffer =*/ VK_NULL_HANDLE,
        /*.occlusionQueryEnable =*/ VK_FALSE,
        /*.queryFlags =*/ 0,
        /*.pipelineStatistics =*/ 0,
    };

    const VkCommandBufferBeginInfo cmdBufInfo = {
        /*.sType =*/ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        /*.pNext =*/ NULL,
        /*.flags =*/ 0,
        /*.pInheritanceInfo =*/ &cmdBufHInfo,
    };

    const VkClearValue clear_values[2] = {
        {/*.color.float32 =*/ {0.2f, 0.2f, 0.2f, 0.2f}},
        {/*.depthStencil =*/ {vc->depthStencil, 0}},
    };

    VkRect2D vkrect = {};
    vkrect.offset.x = 0;
    vkrect.offset.y = 0;
    vkrect.extent.width = vc->width;
    vkrect.extent.height = vc->height;

    const VkRenderPassBeginInfo rp_begin = {
        /*.sType =*/ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        /*.pNext =*/ NULL,
        /*.renderPass =*/ vc->renderPass,
        /*.framebuffer =*/ vc->framebuffers[vc->currentBuffer],
        /*.renderArea =*/ vkrect,
        /*.clearValueCount =*/ 2,
        /*.pClearValues =*/ clear_values,
    };

    VkResult err;

    err = vkBeginCommandBuffer(vc->drawCmd, &cmdBufInfo);
    ASSERT(!err);

    vkCmdBeginRenderPass(vc->drawCmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(vc->drawCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vc->pipeline);
    vkCmdBindDescriptorSets(
            vc->drawCmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vc->pipelineLayout,
            0,
            1,
            &vc->descSet,
            0,
            NULL);

    VkViewport viewport;
    memset(&viewport, 0, sizeof(viewport));
    viewport.height = (float)vc->height;
    viewport.width = (float)vc->width;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
    vkCmdSetViewport(vc->drawCmd, 0, 1, &viewport);

    VkRect2D scissor;
    memset(&scissor, 0, sizeof(scissor));
    scissor.extent.width = vc->width;
    scissor.extent.height = vc->height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(vc->drawCmd, 0, 1, &scissor);

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(vc->drawCmd, VERTEX_BUFFER_BIND_ID, 1, &vc->vertices.buf, offsets);

    vkCmdDraw(vc->drawCmd, 3, 1, 0, 0);
    vkCmdEndRenderPass(vc->drawCmd);

    VkImageMemoryBarrier prePresentBarrier = {
        /*.sType =*/                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        /*.pNext =*/                NULL,
        /*.srcAccessMask =*/        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        /*.dstAccessMask =*/        VK_ACCESS_MEMORY_READ_BIT,
        /*.oldLayout =*/            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        /*.newLayout =*/            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        /*.srcQueueFamilyIndex =*/  VK_QUEUE_FAMILY_IGNORED,
        /*.dstQueueFamilyIndex =*/  VK_QUEUE_FAMILY_IGNORED,
        /*.image =*/                vc->buffers[vc->currentBuffer].image,
        /*.subresourceRange =*/     {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    };

    VkImageMemoryBarrier *pmemoryBarrier = &prePresentBarrier;
    vkCmdPipelineBarrier(
            vc->drawCmd,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0,
            NULL,
            0,
            NULL,
            1,
            pmemoryBarrier);

    err = vkEndCommandBuffer(vc->drawCmd);
    ASSERT(!err);
}


VulkanContext *VulkanSetup(SDL_Window **window)
{
    VulkanContext *vc = (VulkanContext *)malloc(sizeof(VulkanContext));;
    memset(vc, 0, sizeof(VulkanContext));

    VulkanVertices vertices;

    VkSurfaceKHR surface;
    //bool prepared;
	bool useStagingBuffer = false;

    VkInstance inst;
    VkPhysicalDevice gpu;
    VkDevice device;
    VkQueue queue;
    VkPhysicalDeviceProperties gpuProps;
    VkQueueFamilyProperties *queueProps;

    VkDebugReportCallbackEXT msg_callback;
    PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback;
    PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback;
	PFN_vkDebugReportMessageEXT DebugReportMessage;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR
        fpGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR
        fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR
        fpGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR
        fpGetPhysicalDeviceSurfacePresentModesKHR;
    PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
    PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
    PFN_vkQueuePresentKHR fpQueuePresentKHR;
    uint32_t swapchainImageCount;
    VkSwapchainKHR swapchain = {};

	VkCommandPool cmdPool;

	//uint32_t graphicsQueueNodeIndex;
    uint32_t enabledExtensionCount = 0;
    uint32_t enabledLayerCount = 0;
    char *extensionNames[64];
	char *deviceValidationLayers[64];

    VkResult err;
    uint32_t instanceExtensionCount = 0;
    uint32_t instanceLayerCount = 0;
    uint32_t deviceValidationLayerCount = 0;
    char **instanceValidationLayers = NULL;
	VkBool32 validationFound = 0;
	bool useBreak = false;

    bool quit;
    uint32_t currentBuffer;
	uint32_t queue_count;

    VkFormat format;
	VkColorSpaceKHR colorSpace;

    VkCommandBuffer setupCmd = {}; // Command Buffer for initialization commands
    VkCommandBuffer drawCmd = {};  // Command Buffer for drawing commands
    VkPipelineLayout pipelineLayout = {};
    VkDescriptorSetLayout descLayout = {};
    VkPipelineCache pipelineCache = {};
    VkRenderPass renderPass = {};
    VkPipeline pipeline = {};

    VkShaderModule vertShaderModule = {};
    VkShaderModule fragShaderModule = {};

    VkDescriptorPool descPool = {};
    VkDescriptorSet descSet = {};

    VkFramebuffer *framebuffers;
    VkPhysicalDeviceMemoryProperties memoryProperties = {};

    int32_t curFrame;
    int32_t frameCount = 0;
    uint32_t width = 0;
    uint32_t height = 0;

    struct {
        VkFormat format;

        VkImage image;
        VkDeviceMemory mem;
        VkImageView view;
    } depth;

    struct TextureObject textures[DEMO_TEXTURE_COUNT];

    char *instance_validation_layers_alt1[] = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    char *instance_validation_layers_alt2[] = {
        "VK_LAYER_GOOGLE_threading",     "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_device_limits", "VK_LAYER_LUNARG_object_tracker",
        "VK_LAYER_LUNARG_image",         "VK_LAYER_LUNARG_core_validation",
        "VK_LAYER_LUNARG_swapchain",     "VK_LAYER_GOOGLE_unique_objects"
	};

	err = vkEnumerateInstanceLayerProperties(&instanceLayerCount, NULL);
	ASSERT(!err);

	instanceValidationLayers = instance_validation_layers_alt1;
	if (instanceLayerCount > 0)
    {
		VkLayerProperties *instance_layers =
			(VkLayerProperties *)malloc(sizeof (VkLayerProperties) * instanceLayerCount);
		err = vkEnumerateInstanceLayerProperties(&instanceLayerCount, instance_layers);
		ASSERT(!err);

		validationFound = VulkanCheckLayers(
				ARRAY_SIZE(instance_validation_layers_alt1),
				instanceValidationLayers,
                instanceLayerCount,
				instance_layers);

		if (validationFound)
        {
			enabledLayerCount = ARRAY_SIZE(instance_validation_layers_alt1);
			deviceValidationLayers[0] = "VK_LAYER_LUNARG_standard_validation";
			deviceValidationLayerCount = 1;
		}
        else
        {
			// use alternative set of validation layers
			instanceValidationLayers = instance_validation_layers_alt2;
			enabledLayerCount = ARRAY_SIZE(instance_validation_layers_alt2);
			validationFound = VulkanCheckLayers(
					ARRAY_SIZE(instance_validation_layers_alt2),
					instanceValidationLayers,
                    instanceLayerCount,
					instance_layers);
			deviceValidationLayerCount = ARRAY_SIZE(instance_validation_layers_alt2);
			for (uint32_t i = 0; i < deviceValidationLayerCount; i++)
            {
				deviceValidationLayers[i] = instanceValidationLayers[i];
			}
		}
		free(instance_layers);
	}

	if (!validationFound)
    {
		ERROR_PRINT("vkEnumerateInstanceLayerProperties failed to find"
					"required validation layer.\n\n"
					"Please look at the Getting Started guide for additional "
					"information.\n"
					"vkCreateInstance Failure");
	}

	bool validate = true;

	/* Look for instance extensions */
	VkBool32 surfaceExtFound = 0;
	VkBool32 platformSurfaceExtFound = 0;
	memset(extensionNames, 0, sizeof(extensionNames));

	err = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
	ASSERT(err == VK_SUCCESS);

	if (instanceExtensionCount > 0)
    {
		VkExtensionProperties *instance_extensions =
			(VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
		err = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, instance_extensions);
		ASSERT(err == VK_SUCCESS);
		for (uint32_t i = 0; i < instanceExtensionCount; i++)
        {
			printf("%s\n", instance_extensions[i].extensionName);
			if (!strcmp(VK_KHR_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName))
            {
				surfaceExtFound = 1;
				extensionNames[enabledExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
			}

			if (!strcmp(PLATFORM_SURFACE_EXTENSION_NAME, instance_extensions[i].extensionName))
            {
				platformSurfaceExtFound = 1;
				extensionNames[enabledExtensionCount++] = PLATFORM_SURFACE_EXTENSION_NAME;
			}

			if (!strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, instance_extensions[i].extensionName))
            {
				if (validate)
                {
					extensionNames[enabledExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
				}
			}

			ASSERT(enabledExtensionCount < 64);
		}

		free(instance_extensions);
	}

	/* I GOT THROUGH HERE */

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

	const VkApplicationInfo app = {
		/*.sType =*/ VK_STRUCTURE_TYPE_APPLICATION_INFO,
		/*.pNext =*/ NULL,
		/*.pApplicationName =*/ "HELLO",
		/*.applicationVersion =*/ 0,
		/*.pEngineName =*/ "HELLO",
		/*.engineVersion =*/ 0,
		/*.apiVersion =*/ VK_API_VERSION_1_0,
	};

	VkInstanceCreateInfo inst_info = {
		/*.sType =*/ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		/*.pNext =*/ NULL,
		/*.flags =*/ NULL,
		/*.pApplicationInfo =*/ &app,
		/*.enabledLayerCount =*/ enabledLayerCount,
		/*.ppEnabledLayerNames =*/ (const char *const *)instanceValidationLayers,
		/*.enabledExtensionCount =*/ enabledExtensionCount,
		/*.ppEnabledExtensionNames =*/ (const char *const *)extensionNames,
	};

	uint32_t gpuCount;

	err = vkCreateInstance(&inst_info, NULL, &inst);
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

    /* Make initial call to query gpuCount, then second call for gpu info*/
    err = vkEnumeratePhysicalDevices(inst, &gpuCount, NULL);
    ASSERT(!err && gpuCount > 0);

	if (gpuCount > 0)
    {
		VkPhysicalDevice *pysicalDevices =
			(VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * gpuCount);
		err = vkEnumeratePhysicalDevices(inst, &gpuCount, pysicalDevices);
		ASSERT(err == VK_SUCCESS);
		/* For tri demo we just grab the first physical device */
		gpu = pysicalDevices[0];
		free(pysicalDevices);
	} 
    else
    {
		PAUSE_HERE("vkEnumeratePhysicalDevices reported zero accessible devices."
				"\n\nDo you have a compatible Vulkan installable client"
				" driver (ICD) installed?\nPlease look at the Getting Started"
				" guide for additional information.\n"
				"vkEnumeratePhysicalDevices Failure");
	}

	/* Look for validation layers */
	if (validate)
    {
		validationFound = 0;
		enabledLayerCount = 0;
		uint32_t deviceLayerCount = 0;
		err =
			vkEnumerateDeviceLayerProperties(gpu, &deviceLayerCount, NULL);
		ASSERT(err == VK_SUCCESS);

		if (deviceLayerCount > 0)
        {
			VkLayerProperties *deviceLayers =
				(VkLayerProperties *)malloc(sizeof (VkLayerProperties) * deviceLayerCount);
			err = vkEnumerateDeviceLayerProperties(gpu, &deviceLayerCount, deviceLayers);
			ASSERT(err == VK_SUCCESS);

			validationFound = VulkanCheckLayers(
                    deviceValidationLayerCount,
					deviceValidationLayers,
					deviceLayerCount,
					deviceLayers);
			enabledLayerCount = deviceValidationLayerCount;

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

	/* Look for device extensions */
	uint32_t deviceExtensionCount = 0;
	VkBool32 swapchainExtFound = 0;
	enabledExtensionCount = 0;
	memset(extensionNames, 0, sizeof(extensionNames));

	err = vkEnumerateDeviceExtensionProperties(gpu, NULL, &deviceExtensionCount, NULL);
	ASSERT(err == VK_SUCCESS);

	if (deviceExtensionCount > 0)
	{
		VkExtensionProperties *deviceExtensions =
			(VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);
		err = vkEnumerateDeviceExtensionProperties( gpu, NULL, &deviceExtensionCount, deviceExtensions);
		ASSERT(err == VK_SUCCESS);

		for (uint32_t i = 0; i < deviceExtensionCount; i++)
        {
			if (!strcmp(VK_KHR_SWAPCHAIN_EXTENSION_NAME, deviceExtensions[i].extensionName))
			{
				swapchainExtFound = 1;
				extensionNames[enabledExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
			}

			ASSERT(enabledExtensionCount < 64);
		}

		free(deviceExtensions);
	}

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

	if (validate)
	{
        CreateDebugReportCallback =
            (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(inst, "vkCreateDebugReportCallbackEXT");
        DestroyDebugReportCallback =
            (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(inst, "vkDestroyDebugReportCallbackEXT");
        DebugReportMessage =
            (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(inst, "vkDebugReportMessageEXT");

        if (!CreateDebugReportCallback)
        {
            PAUSE_HERE("GetProcAddr: Unable to find vkCreateDebugReportCallbackEXT\n"
                       "vkGetProcAddr Failure");
        }

        if (!DestroyDebugReportCallback) {
            PAUSE_HERE("GetProcAddr: Unable to find vkDestroyDebugReportCallbackEXT\n"
                       "vkGetProcAddr Failure");
        }

        if (!DebugReportMessage)
        {
            PAUSE_HERE("GetProcAddr: Unable to find vkDebugReportMessageEXT\n"
                       "vkGetProcAddr Failure");
        }

		VkDebugReportCallbackCreateInfoEXT dbgCreateInfo;
		dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		dbgCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		dbgCreateInfo.pfnCallback = useBreak ? BreakCallback : dbgFunc;
		dbgCreateInfo.pUserData = NULL;
		dbgCreateInfo.pNext = NULL;
		err = CreateDebugReportCallback(inst, &dbgCreateInfo, NULL, &msg_callback);
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

	// Having these GIPA queries of device extension entry points both
	// BEFORE and AFTER vkCreateDevice is a good test for the loader
	GET_INSTANCE_PROC_ADDR(inst, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(inst, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(inst, GetPhysicalDeviceSurfacePresentModesKHR);
	GET_INSTANCE_PROC_ADDR(inst, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(inst, CreateSwapchainKHR);
	GET_INSTANCE_PROC_ADDR(inst, DestroySwapchainKHR);
	GET_INSTANCE_PROC_ADDR(inst, GetSwapchainImagesKHR);
	GET_INSTANCE_PROC_ADDR(inst, AcquireNextImageKHR);
	GET_INSTANCE_PROC_ADDR(inst, QueuePresentKHR);

	vkGetPhysicalDeviceProperties(gpu, &gpuProps);

	// Query with NULL data to get count
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, NULL);

	queueProps = (VkQueueFamilyProperties *)malloc(queue_count * sizeof(VkQueueFamilyProperties));
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, queueProps);
	ASSERT(queue_count >= 1);

	b32 foundValidGraphics = false;
	for (memory_index i = 0; i < queue_count && foundValidGraphics == false; i++)
	{
		if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			foundValidGraphics = true;
		}
	}

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(gpu, &features);

	if (!features.shaderClipDistance)
	{
		PAUSE_HERE("Required device feature `shaderClipDistance` not supported\n"
				   "GetPhysicalDeviceFeatures failure");
	}

	// Graphics queue and MemMgr queue can be separate.
	// TODO: Add support for separate queues, including synchronization,
	//       and appropriate tracking for QueueSubmit

	/* init swap chain */

	// Create a WSI surface for the window:
	PlatformSurfaceCreateInfo createInfo;
	createInfo.sType = PLATFORM_SURFACE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);

	SDL_GetWindowWMInfo(*window, &info);
	PLATFORM_ASSIGN_SURFACEINFO;
	err = PlatformCreateSurface(inst, &createInfo, NULL, &surface);

	// Iterate over each queue to learn whether it supports presenting:
	VkBool32 *supportsPresent = (VkBool32 *)malloc(queue_count * sizeof(VkBool32));
	for (memory_index i = 0; i < queue_count; i++)
	{
        /* TODO: safecast */
		fpGetPhysicalDeviceSurfaceSupportKHR(gpu, (u32)i, surface, &supportsPresent[i]);
	}

	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	for (memory_index i = 0; i < queue_count; i++)
	{
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
		{
			if (graphicsQueueNodeIndex == UINT32_MAX)
			{
                /* TODO: safecast */
				graphicsQueueNodeIndex = (u32)i;
			}

			if (supportsPresent[i] == VK_TRUE)
			{
				graphicsQueueNodeIndex = (u32)i;
				presentQueueNodeIndex = (u32)i;
				break;
			}
		}
	}

	if (presentQueueNodeIndex == UINT32_MAX)
    {
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (memory_index i = 0; i < queue_count; ++i)
		{
			if (supportsPresent[i] == VK_TRUE) {
				presentQueueNodeIndex = (u32)i;
				break;
			}
		}
	}
	free(supportsPresent);

	// Generate error if could not find both a graphics and a present queue
	if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
	{
		PAUSE_HERE("Could not find a graphics and a present queue\n"
				"Swapchain Initialization Failure");
	}

	// TODO: Add support for separate queues, including presentation,
	//       synchronization, and appropriate tracking for QueueSubmit.
	// NOTE: While it is possible for an application to use a separate graphics
	//       and a present queues, this demo program assumes it is only using
	//       one:
	if (graphicsQueueNodeIndex != presentQueueNodeIndex)
    {
		PAUSE_HERE("Could not find a common graphics and a present queue\n"
				"Swapchain Initialization Failure");
	}

	/* init device */
    float queue_priorities[1] = {0.0};
    const VkDeviceQueueCreateInfo queueInfo = {
        /*.sType =*/ 			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        /*.pNext =*/ 			NULL,
		/*.flags =*/			false,
        /*.queueFamilyIndex =*/ graphicsQueueNodeIndex,
        /*.queueCount =*/ 		1,
        /*.pQueuePriorities =*/ queue_priorities};

    VkPhysicalDeviceFeatures deviceFeatures = { /*.shaderClipDistance =*/ VK_TRUE, };

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

    err = vkCreateDevice(gpu, &deviceInfo, NULL, &device);
	ASSERT(err == VK_SUCCESS);
	/* END INIT DEVICE */

    vkGetDeviceQueue(device, graphicsQueueNodeIndex, 0, &queue);

    // Get the list of VkFormat's that are supported:
    uint32_t formatCount;
    err = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, NULL);
    ASSERT(err == VK_SUCCESS);
    VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    err = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, surfFormats);
    ASSERT(err == VK_SUCCESS);
    // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
    // the surface has no preferred format.  Otherwise, at least one
    // supported format will be returned.
    if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
	{
        format = VK_FORMAT_B8G8R8A8_UNORM;
    }
	else
	{
        ASSERT(formatCount >= 1);
        format = surfFormats[0].format;
    }
    colorSpace = surfFormats[0].colorSpace;

    quit = false;
    curFrame = 0;

    // Get Memory information and properties
	vkGetPhysicalDeviceMemoryProperties(gpu, &memoryProperties);

    /* Prepare stuff */
    const VkCommandPoolCreateInfo cmdPool_info = {
        /*.sType =*/ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        /*.pNext =*/ NULL,
        /*.flags =*/ VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        /*.queueFamilyIndex =*/ graphicsQueueNodeIndex,
    };
    err = vkCreateCommandPool(device, &cmdPool_info, NULL, &cmdPool);
    ASSERT(err == VK_SUCCESS);

    const VkCommandBufferAllocateInfo cmd = {
        /*.sType =*/ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        /*.pNext =*/ NULL,
        /*.commandPool =*/ cmdPool,
        /*.level =*/ VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        /*.commandBufferCount =*/ 1,
    };
    err = vkAllocateCommandBuffers(device, &cmd, &drawCmd);

    /* prepare buffers */
    VkSwapchainKHR oldSwapchain = swapchain;

    // Check the surface capabilities and formats
    VkSurfaceCapabilitiesKHR surfCapabilities;
    err = fpGetPhysicalDeviceSurfaceCapabilitiesKHR( gpu, surface, &surfCapabilities);
    ASSERT(err == VK_SUCCESS);

    uint32_t presentModeCount;
    err = fpGetPhysicalDeviceSurfacePresentModesKHR( gpu, surface, &presentModeCount, NULL);
    ASSERT(err == VK_SUCCESS);

    VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
    ASSERT(presentModes);

    err = fpGetPhysicalDeviceSurfacePresentModesKHR( gpu, surface, &presentModeCount, presentModes);
    ASSERT(err == VK_SUCCESS);

    VkExtent2D swapchainExtent;
    // width and height are either both -1, or both not -1.
    if (surfCapabilities.currentExtent.width == (uint32_t)-1)
    {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
        swapchainExtent.width = width;
        swapchainExtent.height = height;
    }
    else
    {
        // If the surface size is defined, the swap chain size must match
        swapchainExtent = surfCapabilities.currentExtent;
        width = surfCapabilities.currentExtent.width;
        height = surfCapabilities.currentExtent.height;
    }

    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

    // Determine the number of VkImage's to use in the swap chain (we desire to
    // own only 1 image at a time, besides the images being displayed and
    // queued for display):
    uint32_t desiredNumberOfSwapchainImages = surfCapabilities.minImageCount + 1;
    if ((surfCapabilities.maxImageCount > 0) &&
            (desiredNumberOfSwapchainImages > surfCapabilities.maxImageCount))
    {
        // Application must settle for fewer images than desired:
        desiredNumberOfSwapchainImages = surfCapabilities.maxImageCount;
    }

    /* XXX: This used to be VkSurfaceTransformFlagsKHR */
    VkSurfaceTransformFlagBitsKHR preTransform;
    if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else
    {
        preTransform = surfCapabilities.currentTransform;
    }

    const VkSwapchainCreateInfoKHR swapchainInfo = {
        /*.sType =*/                 VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        /*.pNext =*/                 NULL,
        /*.flags =*/                 false,
        /*.surface =*/               surface,
        /*.minImageCount =*/         desiredNumberOfSwapchainImages,
        /*.imageFormat =*/           format,
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

    err = fpCreateSwapchainKHR(device, &swapchainInfo, NULL, &swapchain);
    ASSERT(err == VK_SUCCESS);

    // If we just re-created an existing swapchain, we should destroy the old
    // swapchain at this point.
    // Note: destroying the swapchain also cleans up all its associated
    // presentable images once the platform is done with them.
    if (oldSwapchain != VK_NULL_HANDLE)
    {
        fpDestroySwapchainKHR(device, oldSwapchain, NULL);
    }

    err = fpGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, NULL);
    ASSERT(err == VK_SUCCESS);

    VkImage *swapchainImages = (VkImage *)malloc(swapchainImageCount * sizeof(VkImage));
    ASSERT(swapchainImages);
    err = fpGetSwapchainImagesKHR(device, swapchain, &swapchainImageCount, swapchainImages);
    ASSERT(err == VK_SUCCESS);

    vc->buffers = (SwapchainBuffers *)malloc(sizeof(SwapchainBuffers) * swapchainImageCount);
    ASSERT(vc->buffers);

    uint32_t i;
    for (memory_index i = 0; i < swapchainImageCount; i++)
    {

        vc->buffers[i].image = swapchainImages[i];

        VkImageViewCreateInfo colorAttachmentView = {
            /*.sType =*/            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            /*.pNext =*/            NULL,
            /*.flags =*/            0,
            /* XXX: this is added by me. not sure if it will work */
            /*.image =*/            swapchainImages[i],
            /*.viewType =*/         VK_IMAGE_VIEW_TYPE_2D,
            /*.format =*/           format,
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

        // Render loop will expect image to have been used before and in
        // VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        // layout and will change to COLOR_ATTACHMENT_OPTIMAL, so init the image
        // to that state
        VulkanSetImageLayout(
                &device,
                &setupCmd,
                &cmdPool,
                vc->buffers[i].image,
                VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                (VkAccessFlagBits)0);

        colorAttachmentView.image = vc->buffers[i].image;
        err = vkCreateImageView(device, &colorAttachmentView, NULL, &vc->buffers[i].view);
        ASSERT(!err);
    }

    currentBuffer = 0;

    if (NULL != presentModes)
    {
        free(presentModes);
    }
    /* end prepare buffers */

    /* start prepare depth */
    const VkFormat depthFormat = VK_FORMAT_D16_UNORM;
    VkImageCreateInfo image = {};
    image.sType =                    VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image.pNext =                    NULL;
    image.flags =                    0;
    image.imageType =                VK_IMAGE_TYPE_2D;
    image.format =                   depthFormat;
    image.extent =                   VkExtent3D{width, height, 1};
    image.mipLevels =                1;
    image.arrayLayers =              1;
    image.samples =                  VK_SAMPLE_COUNT_1_BIT;
    image.tiling =                   VK_IMAGE_TILING_OPTIMAL;
    image.usage =                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkMemoryAllocateInfo memAlloc = {
        /*.sType =*/            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        /*.pNext =*/            NULL,
        /*.allocationSize =*/   0,
        /*.memoryTypeIndex =*/  0,
    };

    VkImageViewCreateInfo view = {
        /*.sType =*/                VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        /*.pNext =*/                NULL,
        /*.flags =*/                0,
        /*.image =*/                VK_NULL_HANDLE,
        /*.viewType =*/             VK_IMAGE_VIEW_TYPE_2D,
        /*.format =*/               depthFormat,
        /*.components = */          {},
        /*.subresourceRange =*/     { /*.aspectMask =*/     VK_IMAGE_ASPECT_DEPTH_BIT,
                                      /*.baseMipLevel =*/   0,
                                      /*.levelCount =*/     1,
                                      /*.baseArrayLayer =*/ 0,
                                      /*.layerCount =*/     1
                                    },
    };

    depth.format = depthFormat;

    /* create image */
    err = vkCreateImage(device, &image, NULL, &depth.image);
    ASSERT(err == VK_SUCCESS);

    /* get memory requirements for this object */
    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(device, depth.image, &memReqs);

    /* select memory size and type */
    memAlloc.allocationSize = memReqs.size;
    uint32_t typeBits = memReqs.memoryTypeBits;
    VkFlags requirements_mask = 0; /*no requirements */
    uint32_t *typeIndex = &memAlloc.memoryTypeIndex;

    bool pass = false;
    // Search memtypes to find first index with those properties
    for (uint32_t i = 0; i < 32; i++)
    {
        if ((typeBits & 1) == 1)
        {
            // Type is available, does it match user properties?
            if ((memoryProperties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
            {
                *typeIndex = i;
                pass = true;
                break;
            }
        }
        typeBits >>= 1;
    }
    // false = No memory types matched
    ASSERT(pass);

    /* allocate memory */
    err = vkAllocateMemory(device, &memAlloc, NULL, &depth.mem);
    ASSERT(err == VK_SUCCESS);

    /* bind memory */
    err = vkBindImageMemory(device, depth.image, depth.mem, 0);
    ASSERT(err == VK_SUCCESS);

    VulkanSetImageLayout(
            &device,
            &setupCmd,
            &cmdPool,
            depth.image,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            (VkAccessFlagBits)0);

    /* create image view */
    view.image = depth.image;
    err = vkCreateImageView(device, &view, NULL, &depth.view);
    ASSERT(err == VK_SUCCESS);

    /* end prepare depth */

    /* start prepare uniform buffer */
    struct {
        VkBuffer buf;
        VkDeviceMemory mem;
        VkBufferCreateInfo bufInfo;
        VkDescriptorBufferInfo bufferInfo;
    } uniformData;

    {
        VkBufferCreateInfo bufInfo = {};
        bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufInfo.pNext = NULL;
        bufInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufInfo.size = sizeof(UniformBufferObject);
        bufInfo.queueFamilyIndexCount = 0;
        bufInfo.pQueueFamilyIndices = NULL;
        bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufInfo.flags = 0;
        err = vkCreateBuffer(device, &bufInfo, NULL, &uniformData.buf);
        ASSERT(err == VK_SUCCESS);

        uniformData.bufferInfo.buffer = uniformData.buf;
        uniformData.bufferInfo.offset = 0;
        uniformData.bufferInfo.range = sizeof(UniformBufferObject);

        VkMemoryRequirements memReqs;
        vkGetBufferMemoryRequirements(device, uniformData.buf, &memReqs);

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext = NULL;
        allocInfo.memoryTypeIndex = 0;

        allocInfo.allocationSize = memReqs.size;
        pass = AvailableMemoryTypeFromProperties(
                &memoryProperties,
                memReqs.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &allocInfo.memoryTypeIndex);
        ASSERT(pass);

        err = vkAllocateMemory(device, &allocInfo, NULL, &uniformData.mem);
        ASSERT(err == VK_SUCCESS);

        UniformBufferObject ubo = {};
        void *data;
        err = vkMapMemory(device, uniformData.mem, 0, memReqs.size, 0, &data);
        ASSERT(err == VK_SUCCESS);
        memcpy(data, &ubo, sizeof(UniformBufferObject));

        vkUnmapMemory(device, uniformData.mem);

        err = vkBindBufferMemory(device, uniformData.buf, uniformData.mem, 0);
        ASSERT(err == VK_SUCCESS);
    }

    /* end prepare uniform buffer */

    /* start prepare texture */
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    VkFormatProperties props = {};
    const uint32_t texColors[DEMO_TEXTURE_COUNT][2] = { {0xffff0000, 0xff00ff00}, };

    vkGetPhysicalDeviceFormatProperties(gpu, tex_format, &props);

    for (i = 0; i < DEMO_TEXTURE_COUNT; i++)
    {
        if ((props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) && !useStagingBuffer)
        {
            /* Device can texture using linear textures */
            VulkanPrepareTextureImage(
                    &device,
                    &setupCmd,
                    &cmdPool,
                    &memoryProperties,
                    texColors[i],
                    &textures[i],
                    VK_IMAGE_TILING_LINEAR,
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        }
        else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
        {
            /* Must use staging buffer to copy linear texture to optimized */
            struct TextureObject stagingTexture;

            memset(&stagingTexture, 0, sizeof(stagingTexture));
            VulkanPrepareTextureImage(
                    &device,
                    &setupCmd,
                    &cmdPool,
                    &memoryProperties,
                    texColors[i],
                    &stagingTexture,
                    VK_IMAGE_TILING_LINEAR,
                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

            VulkanPrepareTextureImage(
                    &device,
                    &setupCmd,
                    &cmdPool,
                    &memoryProperties,
                    texColors[i],
                    &textures[i],
                    VK_IMAGE_TILING_OPTIMAL,
                    (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT),
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            VulkanSetImageLayout(
                    &device,
                    &setupCmd,
                    &cmdPool,
                    stagingTexture.image,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    stagingTexture.imageLayout,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    (VkAccessFlagBits)0);

            VulkanSetImageLayout(
                    &device,
                    &setupCmd,
                    &cmdPool,
                    textures[i].image,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    textures[i].imageLayout,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    (VkAccessFlagBits)0);

            VkImageCopy copyRegion = {};
            copyRegion.srcSubresource =    {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
            copyRegion.srcOffset =         {0, 0, 0};
            copyRegion.dstSubresource =    {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
            copyRegion.dstOffset =         {0, 0, 0};

            /* TODO: use safe casting */
            copyRegion.extent =            {uint32(stagingTexture.texWidth),
                                            uint32(stagingTexture.texHeight),
                                            1};

            vkCmdCopyImage(
                    setupCmd, stagingTexture.image,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, textures[i].image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

            VulkanSetImageLayout(
                    &device,
                    &setupCmd,
                    &cmdPool,
                    textures[i].image,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    textures[i].imageLayout,
                    (VkAccessFlagBits)0);

            VulkanFlushInit(&device, &setupCmd, &cmdPool, &queue);
            VulkanDestroyTextureImage(&device, &stagingTexture);
        }
        else
        {
            /* Can't support VK_FORMAT_B8G8R8A8_UNORM !? */
            ASSERT(!"No support for B8G8R8A8_UNORM as texture image format");
        }

        VkSamplerCreateInfo sampler = {};
        sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler.pNext = NULL;
        sampler.magFilter = VK_FILTER_NEAREST;
        sampler.minFilter = VK_FILTER_NEAREST;
        sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler.mipLodBias = 0.0f;
        sampler.anisotropyEnable = VK_FALSE;
        sampler.maxAnisotropy = 1;
        sampler.compareOp = VK_COMPARE_OP_NEVER;
        sampler.minLod = 0.0f;
        sampler.maxLod = 0.0f;
        sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        sampler.unnormalizedCoordinates = VK_FALSE;

        VkImageViewCreateInfo view = {};
        view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view.pNext = NULL;
        view.image = VK_NULL_HANDLE; /* XXX: must be a valid handle instead of VK_NULL_HANDLE ? */
        view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        view.format = tex_format;
        view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
                            VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A,};
        view.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        view.flags = 0;

        /* create sampler */
        err = vkCreateSampler(device, &sampler, NULL, &textures[i].sampler);
        ASSERT(err == VK_SUCCESS);

        /* create image view */
        view.image = textures[i].image;
        err = vkCreateImageView(device, &view, NULL, &textures[i].view);
        ASSERT(err == VK_SUCCESS);
    }
    /* end prepare texture */

    /* start prepare vertices */
    const float vb[3][9] = {
        /*      position                    color           texcoord */
        { -1.0f, -1.0f,  0.25f,   1.0f, 0.0f, 0.0f, 0.0f,   0.0f, 0.0f },
        {  1.0f, -1.0f,  0.25f,   1.0f, 0.0f, 0.0f, 0.0f,   1.0f, 0.0f },
        {  0.0f,  1.0f,  1.0f,    1.0f, 0.0f, 0.0f, 0.0f,   0.5f, 1.0f },
    };

    VkBufferCreateInfo bufInfo = {};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.pNext = NULL;
    bufInfo.size = sizeof(vb);
    bufInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufInfo.flags = 0;

    memAlloc = {};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAlloc.pNext = NULL;
    memAlloc.allocationSize = 0;
    memAlloc.memoryTypeIndex = 0;

    memReqs = {};
    void *data;

    memset(&vertices, 0, sizeof(vertices));

    err = vkCreateBuffer(device, &bufInfo, NULL, &vertices.buf);
    ASSERT(!err);

    vkGetBufferMemoryRequirements(device, vertices.buf, &memReqs);
    ASSERT(!err);

    memAlloc.allocationSize = memReqs.size;
    pass = AvailableMemoryTypeFromProperties(&memoryProperties,
                                             memReqs.memoryTypeBits,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                             &memAlloc.memoryTypeIndex);
    ASSERT(pass);

    err = vkAllocateMemory(device, &memAlloc, NULL, &vertices.mem);
    ASSERT(!err);

    err = vkMapMemory(device, vertices.mem, 0, memAlloc.allocationSize, 0, &data);
    ASSERT(!err);

    memcpy(data, vb, sizeof(vb));

    vkUnmapMemory(device, vertices.mem);

    err = vkBindBufferMemory(device, vertices.buf, vertices.mem, 0);
    ASSERT(!err);

    vertices.vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertices.vi.pNext = NULL;
    vertices.vi.vertexBindingDescriptionCount = 1;
    vertices.vi.pVertexBindingDescriptions = vertices.viBindings;
    vertices.vi.vertexAttributeDescriptionCount = 3;
    vertices.vi.pVertexAttributeDescriptions = vertices.viAttrs;

    vertices.viBindings[0].binding = VERTEX_BUFFER_BIND_ID;
    vertices.viBindings[0].stride = sizeof(vb[0]);
    vertices.viBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    vertices.viAttrs[0].binding = VERTEX_BUFFER_BIND_ID;
    vertices.viAttrs[0].location = 0;
    vertices.viAttrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertices.viAttrs[0].offset = 0;

    vertices.viAttrs[1].binding = VERTEX_BUFFER_BIND_ID;
    vertices.viAttrs[1].location = 1;
    vertices.viAttrs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vertices.viAttrs[1].offset = sizeof(float) * 3;

    vertices.viAttrs[2].binding = VERTEX_BUFFER_BIND_ID;
    vertices.viAttrs[2].location = 2;
    vertices.viAttrs[2].format = VK_FORMAT_R32G32_SFLOAT;
    vertices.viAttrs[2].offset = sizeof(float) * 7;
    /* end prepare vertices*/

#if 1
    /* start prepare descriptor layout */
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutBinding.descriptorCount = DEMO_TEXTURE_COUNT;
    layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBinding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutBinding uboBinding = {};
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    uboBinding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutBinding myDescriptorSetLayoutBinding[] = {uboBinding};

    VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
    descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayout.pNext = NULL;
    descriptorLayout.bindingCount = 1;
    descriptorLayout.pBindings = &myDescriptorSetLayoutBinding[0];

    err = vkCreateDescriptorSetLayout(device, &descriptorLayout, NULL, &descLayout);
    ASSERT(!err);

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = NULL;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descLayout;

    err = vkCreatePipelineLayout(device, &pPipelineLayoutCreateInfo, NULL, &pipelineLayout);
    ASSERT(!err);
    /* end prepare descriptor layout */
#endif

#if 1
    /* start prepare render pass */
    const VkAttachmentDescription attachments[2] = {
        {
            /*.flags =*/            0,
            /*.format =*/           format,
            /*.samples =*/          VK_SAMPLE_COUNT_1_BIT,
            /*.loadOp =*/           VK_ATTACHMENT_LOAD_OP_CLEAR,
            /*.storeOp =*/          VK_ATTACHMENT_STORE_OP_STORE,
            /*.stencilLoadOp =*/    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            /*.stencilStoreOp =*/   VK_ATTACHMENT_STORE_OP_DONT_CARE,
            /*.initialLayout =*/    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            /*.finalLayout =*/      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        },
        {
            /*.flags =*/            0,
            /*.format =*/           depth.format,
            /*.samples =*/          VK_SAMPLE_COUNT_1_BIT,
            /*.loadOp =*/           VK_ATTACHMENT_LOAD_OP_CLEAR,
            /*.storeOp =*/          VK_ATTACHMENT_STORE_OP_DONT_CARE,
            /*.stencilLoadOp =*/    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            /*.stencilStoreOp =*/   VK_ATTACHMENT_STORE_OP_DONT_CARE,
            /*.initialLayout =*/    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            /*.finalLayout =*/      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        },
    };

    const VkAttachmentReference colorReference = {
        /*.attachment =*/   0,
        /*.layout =*/       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkAttachmentReference depthReference = {
        /*.attachment =*/   1,
        /*.layout =*/       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription subpass = {
        /*.flags =*/                    0,
        /*.pipelineBindPoint =*/        VK_PIPELINE_BIND_POINT_GRAPHICS,
        /*.inputAttachmentCount =*/     0,
        /*.pInputAttachments =*/        NULL,
        /*.colorAttachmentCount =*/     1,
        /*.pColorAttachments =*/        &colorReference,
        /*.pResolveAttachments =*/      NULL,
        /*.pDepthStencilAttachment =*/  &depthReference,
        /*.preserveAttachmentCount =*/  0,
        /*.pPreserveAttachments =*/     NULL,
    };

    const VkRenderPassCreateInfo rpInfo = {
        /*.sType =*/            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        /*.pNext =*/            NULL,
        /*.flags =*/            0,
        /*.attachmentCount =*/  2,
        /*.pAttachments =*/     attachments,
        /*.subpassCount =*/     1,
        /*.pSubpasses =*/       &subpass,
        /*.dependencyCount =*/  0,
        /*.pDependencies =*/    NULL,
    };

    err = vkCreateRenderPass(device, &rpInfo, NULL, &renderPass);
    ASSERT(!err);
    /*end prepare render pass*/
#endif

#if 1
    /* start prepare pipeline */
    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    VkPipelineCacheCreateInfo pipelineCacheInfo;

    VkPipelineVertexInputStateCreateInfo vi;
    VkPipelineInputAssemblyStateCreateInfo ia;
    VkPipelineRasterizationStateCreateInfo rs;
    VkPipelineColorBlendStateCreateInfo cb;
    VkPipelineDepthStencilStateCreateInfo ds;
    VkPipelineViewportStateCreateInfo vp;
    VkPipelineMultisampleStateCreateInfo ms;
    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
    VkPipelineDynamicStateCreateInfo dynamicState = {};

    memset(dynamicStateEnables, 0, sizeof(dynamicStateEnables));
    memset(&dynamicState, 0, sizeof(dynamicState));
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables;

    memset(&pipelineCreateInfo, 0, sizeof(pipelineCreateInfo));
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout = pipelineLayout;

    vi = vertices.vi;

    memset(&ia, 0, sizeof(ia));
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    memset(&rs, 0, sizeof(rs));
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.depthClampEnable = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs.depthBiasEnable = VK_FALSE;

    /* If the wide lines feature is not enabled, and no element of the
     * pDynamicStates member of pDynamicState is VK_DYNAMIC_STATE_LINE_WIDTH,
     * the lineWidth member of pRasterizationState must be 1.0
     */
    if (*dynamicState.pDynamicStates != VK_DYNAMIC_STATE_LINE_WIDTH)
    {
        rs.lineWidth = 1.0;
    }

    memset(&cb, 0, sizeof(cb));
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    VkPipelineColorBlendAttachmentState attState[1];
    memset(attState, 0, sizeof(attState));
    attState[0].colorWriteMask = 0xf;
    attState[0].blendEnable = VK_FALSE;
    cb.attachmentCount = 1;
    cb.pAttachments = attState;

    memset(&vp, 0, sizeof(vp));
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    vp.scissorCount = 1;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

    memset(&ds, 0, sizeof(ds));
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.back.failOp = VK_STENCIL_OP_KEEP;
    ds.back.passOp = VK_STENCIL_OP_KEEP;
    ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
    ds.stencilTestEnable = VK_FALSE;
    ds.front = ds.back;

    memset(&ms, 0, sizeof(ms));
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.pSampleMask = NULL;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Two stages: vs and fs
    VkPipelineShaderStageCreateInfo shaderStages[2];
    memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = VulkanPrepareVertexShader(&device, &vertShaderModule);
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = VulkanPrepareFragmentShader(&device, &fragShaderModule);
    shaderStages[1].pName = "main";

    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;

    pipelineCreateInfo.pVertexInputState = &vi;
    pipelineCreateInfo.pInputAssemblyState = &ia;
    pipelineCreateInfo.pRasterizationState = &rs;
    pipelineCreateInfo.pColorBlendState = &cb;
    pipelineCreateInfo.pMultisampleState = &ms;
    pipelineCreateInfo.pViewportState = &vp;
    pipelineCreateInfo.pDepthStencilState = &ds;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    memset(&pipelineCacheInfo, 0, sizeof(pipelineCacheInfo));
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    err = vkCreatePipelineCache(device, &pipelineCacheInfo, NULL, &pipelineCache);
    ASSERT(!err);

    err = vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, NULL, &pipeline);
    ASSERT(!err);

    vkDestroyPipelineCache(device, pipelineCache, NULL);

    vkDestroyShaderModule(device, fragShaderModule, NULL);
    vkDestroyShaderModule(device, vertShaderModule, NULL);

    /* end prepare pipeline */
#endif

    /* start prepare descriptor pool */
#if 0
    const VkDescriptorPoolSize typeCount = {
        /*.type =*/             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        /*.descriptorCount =*/  DEMO_TEXTURE_COUNT,
    };
#else
    const VkDescriptorPoolSize typeCount = {
        /*.type =*/             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        /*.descriptorCount =*/  DEMO_TEXTURE_COUNT,
    };
#endif

    const VkDescriptorPoolCreateInfo descriptorPool = {
        /*.sType =*/            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        /*.pNext =*/            NULL,
        /*.flags =*/            0,
        /*.maxSets =*/          1,
        /*.poolSizeCount =*/    1,
        /*.pPoolSizes =*/       &typeCount,
    };

    err = vkCreateDescriptorPool(device, &descriptorPool, NULL, &descPool);
    ASSERT(!err);
    /* end prepare descriptor pool */


    /* start prepare descriptor set */
    VkDescriptorImageInfo texDescs[DEMO_TEXTURE_COUNT];

    VkDescriptorSetAllocateInfo allocInfo = {
        /*.sType =*/                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        /*.pNext =*/                NULL,
        /*.descriptorPool =*/       descPool,
        /*.descriptorSetCount =*/   1,
        /*.pSetLayouts =*/          &descLayout
    };
    err = vkAllocateDescriptorSets(device, &allocInfo, &descSet);
    ASSERT(!err);

    memset(&texDescs, 0, sizeof(texDescs));
    for (i = 0; i < DEMO_TEXTURE_COUNT; i++)
    {
        texDescs[i].sampler = textures[i].sampler;
        texDescs[i].imageView = textures[i].view;
        texDescs[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    VkWriteDescriptorSet write;
    memset(&write, 0, sizeof(write));
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descSet;
    write.descriptorCount = DEMO_TEXTURE_COUNT;
#if 0
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
#else
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo = &uniformData.bufferInfo;
#endif
    write.pImageInfo = texDescs;

    vkUpdateDescriptorSets(device, 1, &write, 0, NULL);

    /* end prepare descriptor set */

    /* start prepare frame buffers */
    VkImageView attachmentsFrameBuffer[2];
    attachmentsFrameBuffer[1] = depth.view;

    const VkFramebufferCreateInfo fbInfo = {
        /*.sType =*/            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        /*.pNext =*/            NULL,
        /*.flags =*/            0,
        /*.renderPass =*/       renderPass,
        /*.attachmentCount =*/  2,
        /*.pAttachments =*/     attachmentsFrameBuffer,
        /*.width =*/            width,
        /*.height =*/           height,
        /*.layers =*/           1,
    };

    framebuffers = (VkFramebuffer *)malloc(swapchainImageCount * sizeof(VkFramebuffer));
    ASSERT(framebuffers);

    for (i = 0; i < swapchainImageCount; i++)
    {
        attachmentsFrameBuffer[0] = vc->buffers[i].view;
        err = vkCreateFramebuffer(device, &fbInfo, NULL, &framebuffers[i]);
        ASSERT(!err);
    }

    /* end prepare frame buffers */
	//PAUSE_HERE("Prepare complete!!!\n");

    vc->device = device;
    vc->fpAcquireNextImageKHR = fpAcquireNextImageKHR;
    vc->fpQueuePresentKHR = fpQueuePresentKHR;
    vc->swapchain = swapchain;
    vc->currentBuffer = currentBuffer;
    vc->curFrame = curFrame;
    vc->frameCount = frameCount;

    vc->cmdPool = cmdPool;
    vc->setupCmd = setupCmd;
    vc->drawCmd = drawCmd;
    vc->queue = queue;

    vc->renderPass = renderPass;
    vc->framebuffers = framebuffers;

    vc->pipeline = pipeline;
    vc->pipelineLayout = pipelineLayout;

    vc->vertices = vertices;

    vc->width = width;
    vc->height = height;
    vc->depthStencil = 1.0;
    vc->depthIncrement = -0.01f;

    vc->descSet = descSet;

    vc->quit = false;
    return vc;

}

void VulkanRender(VulkanContext *vc)
{
    VkResult err;
    VkSemaphore presentCompleteSemaphore;
    VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo = {
        /*.sType =*/ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        /*.pNext =*/ NULL,
        /*.flags =*/ 0,
    };

    err = vkCreateSemaphore(vc->device, &presentCompleteSemaphoreCreateInfo,
            NULL, &presentCompleteSemaphore);
    ASSERT(!err);

    // Get the index of the next available swapchain image:
    err = vc->fpAcquireNextImageKHR(vc->device,
                                    vc->swapchain,
                                    UINT64_MAX,
                                    presentCompleteSemaphore,
                                    (VkFence)0, // TODO: Show use of fence
                                    &vc->currentBuffer);

    if (err == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // demo->swapchain is out of date (e.g. the window was resized) and
        // must be recreated:
        /* XXX: TODO
         * vulkan_resize(vc);
         */
        PAUSE_HERE("pausing\n");
        VulkanRender(vc);
        vkDestroySemaphore(vc->device, presentCompleteSemaphore, NULL);
        return;
    }
    else if (err == VK_SUBOPTIMAL_KHR)
    {
        // demo->swapchain is not as optimal as it could be, but the platform's
        // presentation engine will still present the image correctly.
    }
    else
    {
        ASSERT(!err);
    }

    // Assume the command buffer has been run on currentBuffer before so
    // we need to set the image layout back to COLOR_ATTACHMENT_OPTIMAL
    VulkanSetImageLayout(
            &vc->device,
            &vc->setupCmd,
            &vc->cmdPool,
            vc->buffers[vc->currentBuffer].image,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            (VkAccessFlagBits)0);

    VulkanFlushInit(
            &vc->device,
            &vc->setupCmd,
            &vc->cmdPool,
            &vc->queue);

    // Wait for the present complete semaphore to be signaled to ensure
    // that the image won't be rendered to until the presentation
    // engine has fully released ownership to the application, and it is
    // okay to render to the image.

    // FIXME/TODO: DEAL WITH VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    VulkanBuildDrawCommand(vc);

    VkFence nullFence = VK_NULL_HANDLE;
    VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submitInfo = {
        /*.sType =*/                VK_STRUCTURE_TYPE_SUBMIT_INFO,
        /*.pNext =*/                NULL,
        /*.waitSemaphoreCount =*/   1,
        /*.pWaitSemaphores =*/      &presentCompleteSemaphore,
        /*.pWaitDstStageMask =*/    &pipeStageFlags,
        /*.commandBufferCount =*/   1,
        /*.pCommandBuffers =*/      &vc->drawCmd,
        /*.signalSemaphoreCount =*/ 0,
        /*.pSignalSemaphores =*/    NULL
    };

    err = vkQueueSubmit(vc->queue, 1, &submitInfo, nullFence);
    ASSERT(err == VK_SUCCESS);

    VkPresentInfoKHR present = {
        /*.sType =*/                VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        /*.pNext =*/                NULL,
        /*.waitSemaphoreCount =*/   0,
        /*.pWaitSemaphores =*/      nullptr,
        /*.swapchainCount =*/       1,
        /*.pSwapchains =*/          &vc->swapchain,
        /*.pImageIndices =*/        &vc->currentBuffer,
        /*.pResults =*/             nullptr
    };

    // TBD/TODO: SHOULD THE "present" PARAMETER BE "const" IN THE HEADER?
    err = vc->fpQueuePresentKHR(vc->queue, &present);

    if (err == VK_ERROR_OUT_OF_DATE_KHR)
    {
        // vc->swapchain is out of date (e.g. the window was resized) and
        // must be recreated:
        ASSERT(!"pausing\n");
        //vulkan_resize(vc);
    }
    else if (err == VK_SUBOPTIMAL_KHR)
    {
        // vc->swapchain is not as optimal as it could be, but the platform's
        // presentation engine will still present the image correctly.
    }
    else
    {
        ASSERT(!err);
    }

    err = vkQueueWaitIdle(vc->queue);
    ASSERT(err == VK_SUCCESS);

    vkDestroySemaphore(vc->device, presentCompleteSemaphore, NULL);

}
#endif
