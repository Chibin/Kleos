#ifndef __MY_VULKAN__
#define __MY_VULKAN__

#include "./vulkan.h"
#include "stddef.h"
#include "../../string.h"
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                               \
    {                                                                          \
        fp##entrypoint =                                                 	   \
            (PFN_vk##entrypoint)vkGetInstanceProcAddr(inst, "vk" #entrypoint); \
        if (fp##entrypoint == nullptr) {                                          \
            PAUSE_HERE("vkGetInstanceProcAddr failed to find vk" #entrypoint   \
                     "vkGetInstanceProcAddr Failure");                         \
        }                                                                      \
    }

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                                  \
    {                                                                          \
        fp##entrypoint =                                                 	   \
            (PFN_vk##entrypoint)vkGetDeviceProcAddr(dev, "vk" #entrypoint);    \
        if (fp##entrypoint == nullptr) {                                    	   \
            PAUSE_HERE("vkGetDeviceProcAddr failed to find vk" #entrypoint     \
                     "vkGetDeviceProcAddr Failure");                           \
        }                                                                      \
	}

#define  ATTACHMENT_COUNT 3
#include "./shader.cpp"
#include "./layers.cpp"
void VulkanRender(VulkanContext *vc, u32 numOfVertices);

static bool AvailableMemoryTypeFromProperties(
        VkPhysicalDeviceMemoryProperties *memoryProperties,
        uint32_t typeBits,
        VkFlags requirementsMask,
        uint32_t *typeIndex)
{
    for (uint32_t i = 0; i < 32; i++)
    {
        if ((typeBits & 1) == 1)
        {
            // Type is available, does it match user properties?
            if ((memoryProperties->memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
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

void VulkanFlushInit(
        VkDevice *device,
        VkCommandBuffer *setupCmd,
        VkCommandPool *cmdPool,
        VkQueue *queue)
{
    VkResult err;

    if (*setupCmd == VK_NULL_HANDLE)
    {
        return;
    }

    err = vkEndCommandBuffer(*setupCmd);
    ASSERT(err == VK_SUCCESS);

    VkFence nullFence = {VK_NULL_HANDLE};
    const VkCommandBuffer cmdBufs[] = {*setupCmd};

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = cmdBufs;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;

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
        vkDestroyImage(*device, texObj->image, nullptr);
        vkFreeMemory(*device, texObj->mem, nullptr);
}

#include "./memory.cpp"
#include "./image.cpp"

void VulkanUpdateVertices(VulkanContext *vc, void *verticesData, VkDeviceSize verticesSize)
{

    vkCmdUpdateBuffer(
            vc->drawCmd,
            vc->vertices.buf,
            0,
            verticesSize,
            verticesData);
}

void VulkanPrepareVertices(
        VulkanContext *vc, VkBuffer *buffer, VkDeviceMemory *mem,
        void *verticesData, VkDeviceSize verticesSize)
{

    VkResult err = {};
    VkPhysicalDeviceMemoryProperties memoryProperties = {};
	vkGetPhysicalDeviceMemoryProperties(vc->gpu, &memoryProperties);

    VkMemoryRequirements memReqs = {};
    void *data;

    VkDeviceSize memSize = VulkanCreateBuffer(
            &vc->device,
            &memoryProperties,
            verticesSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            buffer,
            mem);

    err = vkMapMemory(vc->device, *mem, 0, memSize, 0, &data);
    ASSERT(!err);
    memcpy(data, verticesData, memSize);
    vkUnmapMemory(vc->device, *mem);

}

void VulkanBeginRenderPass(VulkanContext *vc)
{
    const VkCommandBufferInheritanceInfo cmdBufHInfo = {
        /*.sType =*/ VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        /*.pNext =*/ nullptr,
        /*.renderPass =*/ VK_NULL_HANDLE,
        /*.subpass =*/ 0,
        /*.framebuffer =*/ VK_NULL_HANDLE,
        /*.occlusionQueryEnable =*/ VK_FALSE,
        /*.queryFlags =*/ 0,
        /*.pipelineStatistics =*/ 0,
    };

    const VkCommandBufferBeginInfo cmdBufInfo = {
        /*.sType =*/ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        /*.pNext =*/ nullptr,
        /*.flags =*/ 0,
        /*.pInheritanceInfo =*/ &cmdBufHInfo,
    };

    VkClearValue clearValues[2] = {
        {/*.color.float32 =*/ {0.2f, 0.2f, 0.2f, 0.2f}},
        {/*.depthStencil =*/ {vc->depthStencil, 0}},
    };

    VkRect2D vkrect = {};
    vkrect.offset.x = 0;
    vkrect.offset.y = 0;
    vkrect.extent.width = vc->width;
    vkrect.extent.height = vc->height;

    const VkRenderPassBeginInfo rpBegin = {
        /*.sType =*/            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        /*.pNext =*/            nullptr,
        /*.renderPass =*/       vc->renderPass,
        /*.framebuffer =*/      vc->framebuffers[vc->currentBufferIndex],
        /*.renderArea =*/       vkrect,
        /*.clearValueCount =*/  2,
        /*.pClearValues =*/     clearValues,
    };

    VkResult err;
    err = vkBeginCommandBuffer(vc->drawCmd, &cmdBufInfo);
    ASSERT(!err);

    vkCmdBeginRenderPass(vc->drawCmd, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(vc->drawCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, vc->pipeline);
    vkCmdBindDescriptorSets(
            vc->drawCmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            vc->pipelineLayout,
            0,
            1,
            vc->descSet,
            0,
            nullptr);

}

void VulkanSetViewportAndScissor(VulkanContext *vc)
{
    VkViewport viewport = {};
#if 0
    viewport.y = 0;
    viewport.height = (float)vc->height;
#else
    /* Flip the view port so that it behaves similar to OpenGL */
    viewport.y = (float)vc->height;
    viewport.height = -1 * (float)vc->height;
#endif
    viewport.width = (float)vc->width;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
    vkCmdSetViewport(vc->drawCmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.extent.width = vc->width;
    scissor.extent.height = vc->height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(vc->drawCmd, 0, 1, &scissor);
}

void VulkanAddDrawCmd(VulkanContext *vc, VkBuffer *buf, u32 numOfVertices)
{
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(vc->drawCmd, VERTEX_BUFFER_BIND_ID, 1, buf, offsets);

    vkCmdDraw(vc->drawCmd, numOfVertices, 1, 0, 0);
}

void VulkanEndBufferCommands(VulkanContext *vc)
{
    vkCmdEndRenderPass(vc->drawCmd);

    VkImageMemoryBarrier prePresentBarrier = {
        /*.sType =*/                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        /*.pNext =*/                nullptr,
        /*.srcAccessMask =*/        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        /*.dstAccessMask =*/        VK_ACCESS_MEMORY_READ_BIT,
        /*.oldLayout =*/            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        /*.newLayout =*/            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        /*.srcQueueFamilyIndex =*/  VK_QUEUE_FAMILY_IGNORED,
        /*.dstQueueFamilyIndex =*/  VK_QUEUE_FAMILY_IGNORED,
        /*.image =*/                vc->buffers[vc->currentBufferIndex].image,
        /*.subresourceRange =*/     {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    };

    VkImageMemoryBarrier *pmemoryBarrier = &prePresentBarrier;
    vkCmdPipelineBarrier(
            vc->drawCmd,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            pmemoryBarrier);

    ASSERT(vkEndCommandBuffer(vc->drawCmd) == VK_SUCCESS);
}

static void VulkanBuildDrawCommand(struct VulkanContext *vc, u32 numOfVertices)
{
    VkResult err;

    VulkanBeginRenderPass(vc);
    VulkanSetViewportAndScissor(vc);

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(vc->drawCmd, VERTEX_BUFFER_BIND_ID, 1, &vc->vertices.buf, offsets);

    vkCmdDraw(vc->drawCmd, numOfVertices, 1, 0, 0);

    vkCmdEndRenderPass(vc->drawCmd);

    VkImageMemoryBarrier prePresentBarrier = {
        /*.sType =*/                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        /*.pNext =*/                nullptr,
        /*.srcAccessMask =*/        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        /*.dstAccessMask =*/        VK_ACCESS_MEMORY_READ_BIT,
        /*.oldLayout =*/            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        /*.newLayout =*/            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        /*.srcQueueFamilyIndex =*/  VK_QUEUE_FAMILY_IGNORED,
        /*.dstQueueFamilyIndex =*/  VK_QUEUE_FAMILY_IGNORED,
        /*.image =*/                vc->buffers[vc->currentBufferIndex].image,
        /*.subresourceRange =*/     {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
    };

    VkImageMemoryBarrier *pmemoryBarrier = &prePresentBarrier;
    vkCmdPipelineBarrier(
            vc->drawCmd,
            VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            pmemoryBarrier);

    err = vkEndCommandBuffer(vc->drawCmd);
    ASSERT(!err);
}

void VulkanCreateUniformBuffer(
        VkDevice *device,
        VkPhysicalDeviceMemoryProperties *memoryProperties,
        UniformObject *uniformData,
        UniformBufferObject *ubo
        )
{
    VkResult err;

    VulkanCreateBuffer(
            device,
            memoryProperties,
            sizeof(UniformBufferObject),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &uniformData->buf,
            &uniformData->mem);

    uniformData->bufferInfo.buffer = uniformData->buf;
    uniformData->bufferInfo.offset = 0;
    uniformData->bufferInfo.range = sizeof(UniformBufferObject);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(*device, uniformData->buf, &memReqs);

    void *data;
    err = vkMapMemory(*device, uniformData->mem, 0, memReqs.size, 0, &data);
    ASSERT(err == VK_SUCCESS);
    memcpy(data, ubo, sizeof(UniformBufferObject));

    vkUnmapMemory(*device, uniformData->mem);
}

void VulkanUpdateUniformBuffer(VulkanContext *vc, UniformBufferObject *ubo)
{
    UniformObject *uniformData = &vc->uniformData;
    VkResult err = {};
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(vc->device, uniformData->buf, &memReqs);

    void *data;
    err = vkMapMemory(vc->device, uniformData->mem, 0, memReqs.size, 0, &data);
    ASSERT(err == VK_SUCCESS);
    memcpy(data, ubo, sizeof(UniformBufferObject));

    vkUnmapMemory(vc->device, uniformData->mem);

}

#include "./instance.cpp"
#include "./surface.cpp"
#include "./extensions.cpp"
#include "./gpu.cpp"
#include "./device.cpp"
#include "./buffers.cpp"
#include "./layout.cpp"
#include "./pipeline.cpp"
#include "./render_pass.cpp"
#include "./descriptor.cpp"

VulkanContext *VulkanSetup(SDL_Window **window)
{
    VulkanContext *vc = (VulkanContext *)malloc(sizeof(VulkanContext));;
    memset(vc, 0, sizeof(VulkanContext));

    VkDevice device;
    VkQueue queue;

    VkDebugReportCallbackEXT msgCallback;
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

    VkCommandPool cmdPool;

    uint32_t enabledLayerCount = 0;
    char *extensionNames[64];
    char *deviceValidationLayers[64];

    VkResult err;
    uint32_t deviceValidationLayerCount = 0;
    bool useBreak = false;

    uint32_t currentBufferIndex;

    VkColorSpaceKHR colorSpace;

    VkCommandBuffer setupCmd = {}; // Command Buffer for initialization commands
    VkCommandBuffer drawCmd = {};  // Command Buffer for drawing commands

    VkPhysicalDeviceMemoryProperties memoryProperties = {};

    uint32_t width = 0;
    uint32_t height = 0;

    char *instanceValidationLayersAlt1[] = {
        "VK_LAYER_LUNARG_standard_validation"
    };

    char *instanceValidationLayersAlt2[] = {
        "VK_LAYER_GOOGLE_threading",     "VK_LAYER_LUNARG_parameter_validation",
        "VK_LAYER_LUNARG_device_limits", "VK_LAYER_LUNARG_object_tracker",
        "VK_LAYER_LUNARG_image",         "VK_LAYER_LUNARG_core_validation",
        "VK_LAYER_LUNARG_swapchain",     "VK_LAYER_GOOGLE_unique_objects"
	};

    u32 instanceLayerCount = 0;
	err = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
	ASSERT(!err);

    VulkanSetInstanceLayer(
        instanceLayerCount,
        instanceValidationLayersAlt1,
        instanceValidationLayersAlt2,
        ARRAY_SIZE(instanceValidationLayersAlt1),
        ARRAY_SIZE(instanceValidationLayersAlt2),
        deviceValidationLayers,
        &deviceValidationLayerCount,
        &enabledLayerCount);

	bool validate = true; /* This needed to be an app argument */

    u32 enabledExtensionCount = 0;
	memset(extensionNames, 0, sizeof(extensionNames));
    VulkanSetSurfaceExtensionNames(
        extensionNames,
        &enabledExtensionCount,
        validate);

	const VkApplicationInfo app = {
		/*.sType =*/                VK_STRUCTURE_TYPE_APPLICATION_INFO,
		/*.pNext =*/                nullptr,
		/*.pApplicationName =*/     "HELLO",
		/*.applicationVersion =*/   0,
		/*.pEngineName =*/          "HELLO",
		/*.engineVersion =*/        0,
		/*.apiVersion =*/           VK_API_VERSION_1_1,
	};

	VkInstanceCreateInfo instInfo = {
		/*.sType =*/                    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		/*.pNext =*/                    nullptr,
		/*.flags =*/                    0,
		/*.pApplicationInfo =*/         &app,
		/*.enabledLayerCount =*/        enabledLayerCount,
		/*.ppEnabledLayerNames =*/      (const char *const *)deviceValidationLayers,
		/*.enabledExtensionCount =*/    enabledExtensionCount,
		/*.ppEnabledExtensionNames =*/  (const char *const *)extensionNames,
	};

    VkInstance inst;
    VkPhysicalDevice gpu;
    VulkanSetInstance(&inst, &instInfo);
    VulkanSetPhysicalDevice(&inst, &gpu);

    if (validate)
    {
        VulkanSetValidationLayer(
                &gpu,
                deviceValidationLayers,
                deviceValidationLayerCount,
                &enabledLayerCount);
    }

    VulkanSetSwapchainExtensionName(
        &gpu,
        (memset(extensionNames, 0, sizeof(extensionNames)), extensionNames), /* using comma operator for lol */
        &enabledExtensionCount);

	if (validate)
	{
        VulkanGetDebugEXTFunctions(
                &inst,
                CreateDebugReportCallback,
                DestroyDebugReportCallback,
                DebugReportMessage,
                useBreak,
                &msgCallback);
    }

	/* Having these GIPA (get instance proc addr?) queries of device extension entry points both
	 * BEFORE and AFTER vkCreateDevice is a good test for the loader
     */
	GET_INSTANCE_PROC_ADDR(inst, GetPhysicalDeviceSurfaceCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(inst, GetPhysicalDeviceSurfaceFormatsKHR);
	GET_INSTANCE_PROC_ADDR(inst, GetPhysicalDeviceSurfacePresentModesKHR);
	GET_INSTANCE_PROC_ADDR(inst, GetPhysicalDeviceSurfaceSupportKHR);
	GET_INSTANCE_PROC_ADDR(inst, CreateSwapchainKHR);
	GET_INSTANCE_PROC_ADDR(inst, DestroySwapchainKHR);
	GET_INSTANCE_PROC_ADDR(inst, GetSwapchainImagesKHR);
	GET_INSTANCE_PROC_ADDR(inst, AcquireNextImageKHR);
	GET_INSTANCE_PROC_ADDR(inst, QueuePresentKHR);

    vc->fpGetPhysicalDeviceSurfaceSupportKHR = fpGetPhysicalDeviceSurfaceSupportKHR;
    vc->fpAcquireNextImageKHR = fpAcquireNextImageKHR;
    vc->fpQueuePresentKHR = fpQueuePresentKHR;
    vc->fpGetPhysicalDeviceSurfaceCapabilitiesKHR = fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
    vc->fpDestroySwapchainKHR = fpDestroySwapchainKHR;
    vc->fpCreateSwapchainKHR = fpCreateSwapchainKHR;
    vc->fpGetSwapchainImagesKHR = fpGetSwapchainImagesKHR;
    vc->fpGetPhysicalDeviceSurfacePresentModesKHR = fpGetPhysicalDeviceSurfacePresentModesKHR;

	u32 queueCount = 0;
    VkQueueFamilyProperties *queueProps = nullptr;
    VulkanInitQueueProperties(&gpu, &queueCount, &queueProps);

    b32 queueHasGraphicsSupport = false;
	for (memory_index i = 0; i < queueCount; i++)
	{
		if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			queueHasGraphicsSupport = true;
            break;
		}
	}
    ASSERT(queueHasGraphicsSupport);

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(gpu, &features);
	if (!features.shaderClipDistance)
	{
		ASSERT(!"Required device feature `shaderClipDistance` not supported\n"
			    "GetPhysicalDeviceFeatures failure");
	}

	// Graphics queue and MemMgr queue can be separate.
	// TODO: Add support for separate queues, including synchronization,
	//       and appropriate tracking for QueueSubmit

	/* init swap chain ???
     * I don't see any swap chain so far*/

	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;

    VkSurfaceKHR surface;
    VulkanGetSupportedQueueFamily(
        vc,
        &inst,
        &gpu,
        &surface,
        window,
        queueProps,
        &graphicsQueueNodeIndex,
        &presentQueueNodeIndex);

    VulkanInitDevice(
        &device,
        &gpu,
        graphicsQueueNodeIndex,
        enabledLayerCount,
        enabledExtensionCount,
        extensionNames,
        validate,
        deviceValidationLayers);

    vkGetDeviceQueue(device, graphicsQueueNodeIndex, 0, &queue);

    /* Get the list of VkFormat's that are supported */
    u32 formatCount;
    err = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, nullptr);
    ASSERT(err == VK_SUCCESS && formatCount >= 1);
    VkSurfaceFormatKHR *surfFormats =
        (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    err = fpGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &formatCount, surfFormats);
    ASSERT(err == VK_SUCCESS);

    /* If the format list includes just one entry of VK_FORMAT_UNDEFINED,
     * the surface has no preferred format. Otherwise, at least one
     * supported format will be returned.
     */
    vc->format =
        (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) ? 
        VK_FORMAT_R8G8B8A8_UNORM : surfFormats[0].format;
    colorSpace = surfFormats[0].colorSpace;

    const VkCommandPoolCreateInfo cmdPoolInfo = {
        /*.sType =*/ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        /*.pNext =*/ nullptr,
        /*.flags =*/ VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        /*.queueFamilyIndex =*/ graphicsQueueNodeIndex,
    };
    err = vkCreateCommandPool(device, &cmdPoolInfo, nullptr, &cmdPool);
    ASSERT(err == VK_SUCCESS);

    const VkCommandBufferAllocateInfo cmd = {
        /*.sType =*/                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        /*.pNext =*/                nullptr,
        /*.commandPool =*/          cmdPool,
        /*.level =*/                VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        /*.commandBufferCount =*/   1,
    };
    err = vkAllocateCommandBuffers(device, &cmd, &drawCmd);

    VkSwapchainKHR swapchain = {};
    VulkanPrepareSwapchain(vc,
        &setupCmd,
        &cmdPool,
        &gpu,
        &surface,
        &vc->format,
        &swapchain,
        &width,
        &height,
        &device,
        colorSpace,
        &vc->swapchainImageCount,
        &currentBufferIndex);

    VulkanInitDepthBuffer(vc, width, height, &vc->depth, &device, &setupCmd, &cmdPool);

    // Get Memory information and properties
	vkGetPhysicalDeviceMemoryProperties(gpu, &memoryProperties);

    /* start prepare uniform buffer */
    vc->uniformData = {};
    UniformObject *uniformData = &vc->uniformData;
    UniformBufferObject ubo = {};
    VulkanCreateUniformBuffer(
        &device,
        &memoryProperties,
        uniformData,
        &ubo);

    UniformObject *uniformDataFragment = &vc->uniformDataFragment;
    UniformBufferObject uboFrag = {};
    VulkanCreateUniformBuffer(
        &device,
        &memoryProperties,
        uniformDataFragment,
        &uboFrag);
    /* end prepare uniform buffer */

    vc->device = device;
    vc->gpu = gpu;
    vc->fpAcquireNextImageKHR = fpAcquireNextImageKHR;
    vc->fpQueuePresentKHR = fpQueuePresentKHR;
    vc->swapchain = swapchain;
    vc->currentBufferIndex = currentBufferIndex;
    vc->curFrame = 0;
    vc->frameCount = 0;

    vc->cmdPool = cmdPool;
    vc->setupCmd = setupCmd;
    vc->drawCmd = drawCmd;
    vc->queue = queue;

    vc->width = width;
    vc->height = height;
    vc->depthStencil = 1.0;
    vc->memoryProperties = memoryProperties;
    vc->depthIncrement = -0.01f;

    vc->descSet = nullptr;

    vc->quit = false;
    return vc;

}

/* Use the swapchain images by associating swapchain image view with the
 * framebuffer. If you have depth enabled, you will also need to associate the
 * depth view with the frame buffer. Each framebuffer will require a
 * "swapchain image" view and depth view attached
 */
void VulkanInitFrameBuffers(VulkanContext *vc)
{
    VkResult err;
    VkDevice *device = &vc->device;

    /* The amount of frame buffers rely on how many attachments we have in the renderpass */
    VkImageView attachmentsFrameBuffer[ATTACHMENT_COUNT];
    attachmentsFrameBuffer[1] = vc->depth.view;

    const VkFramebufferCreateInfo fbInfo = {
        /*.sType =*/            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        /*.pNext =*/            nullptr,
        /*.flags =*/            0,
        /*.renderPass =*/       vc->renderPass,
        /*.attachmentCount =*/  ATTACHMENT_COUNT,
        /*.pAttachments =*/     attachmentsFrameBuffer,
        /*.width =*/            vc->width,
        /*.height =*/           vc->height,
        /*.layers =*/           1,
    };

    vc->framebuffers = (VkFramebuffer *)malloc(vc->swapchainImageCount * sizeof(VkFramebuffer));
    ASSERT(vc->framebuffers);

    for (memory_index i = 0; i < vc->swapchainImageCount; i++)
    {
        attachmentsFrameBuffer[0] = vc->buffers[i].view;
        attachmentsFrameBuffer[2] = vc->buffers[i].view;
        err = vkCreateFramebuffer(*device, &fbInfo, nullptr, &vc->framebuffers[i]);
        ASSERT(!err);
    }

}

void VulkanEndRender(VulkanContext *vc)
{
    VkResult err = {};
    VkFence nullFence = VK_NULL_HANDLE;
    VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo submitInfo = {
        /*.sType =*/                VK_STRUCTURE_TYPE_SUBMIT_INFO,
        /*.pNext =*/                nullptr,
        /*.waitSemaphoreCount =*/   1,
        /*.pWaitSemaphores =*/      &vc->presentCompleteSemaphore,
        /*.pWaitDstStageMask =*/    &pipeStageFlags,
        /*.commandBufferCount =*/   1,
        /*.pCommandBuffers =*/      &vc->drawCmd,
        /*.signalSemaphoreCount =*/ 0,
        /*.pSignalSemaphores =*/    nullptr
    };

    err = vkQueueSubmit(vc->queue, 1, &submitInfo, nullFence);
    ASSERT(err == VK_SUCCESS);

    VkPresentInfoKHR present = {
        /*.sType =*/                VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        /*.pNext =*/                nullptr,
        /*.waitSemaphoreCount =*/   0,
        /*.pWaitSemaphores =*/      nullptr,
        /*.swapchainCount =*/       1,
        /*.pSwapchains =*/          &vc->swapchain,
        /*.pImageIndices =*/        &vc->currentBufferIndex,
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

    vkDestroySemaphore(vc->device, vc->presentCompleteSemaphore, nullptr);
}

void VulkanPrepareRender(VulkanContext *vc)
{
    VkResult err;
    VkSemaphoreCreateInfo presentCompleteSemaphoreCreateInfo = {
        /*.sType =*/ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        /*.pNext =*/ nullptr,
        /*.flags =*/ 0,
    };

    err = vkCreateSemaphore(vc->device,
                            &presentCompleteSemaphoreCreateInfo,
                            nullptr,
                            &vc->presentCompleteSemaphore);
    ASSERT(!err);

    /* Get the index of the next available swapchain image */
    err = vc->fpAcquireNextImageKHR(vc->device,
                                    vc->swapchain,
                                    UINT64_MAX,
                                    vc->presentCompleteSemaphore,
                                    (VkFence)0, /* may be explore fences? */
                                    &vc->currentBufferIndex);

    if (err == VK_ERROR_OUT_OF_DATE_KHR)
    {
        /* swapchain is out of date (e.g. the window was resized) and
         * must be recreated
         */

        /* XXX: TODO
         * vulkan_resize(vc);
         */
        PAUSE_HERE("pausing\n");
        VulkanRender(vc, 0);
        vkDestroySemaphore(vc->device, vc->presentCompleteSemaphore, nullptr);
        return;
    }
    else if (err == VK_SUBOPTIMAL_KHR)
    {
        /* swapchain is not as optimal as it could be, but the platform's
         * presentation engine will still present the image correctly.
         */
    }
    else
    {
        ASSERT(!err);
    }

    // Assume the command buffer has been run on currentBufferIndex before so
    // we need to set the image layout back to COLOR_ATTACHMENT_OPTIMAL
    VulkanAddPipelineBarrier(
            &vc->device,
            &vc->setupCmd,
            &vc->cmdPool,
            vc->buffers[vc->currentBufferIndex].image,
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

}

void VulkanRender(VulkanContext *vc, u32 numOfVertices)
{
    VulkanPrepareRender(vc);

    // FIXME/TODO: DEAL WITH VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    VulkanBuildDrawCommand(vc, numOfVertices);

    VulkanEndRender(vc);
}

/* If the image path is empty, we assume that we don't need to load the image. */
void VulkanLoadImageToGPU(VulkanContext *vc, VulkanDescriptorSetInfo *vdsi)
{
    stbi_set_flip_vertically_on_load(1);
    b32 useStagingBuffer = false;

    s32 texWidth = 0;
    s32 texHeight = 0;
    s32 texChannels = 0;

    stbi_uc *pixels = nullptr;

    /* image path not empty */
    if (! IsStringEmpty(vdsi->imagePath))
    {
        pixels = stbi_load(vdsi->imagePath,
                           &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        vdsi->textureObj[0].texWidth = texWidth;
        vdsi->textureObj[0].texHeight = texHeight;
        /* stbi does not do padding, so the pitch is the component times
         * the width of the image. The component is 4 because of STBI_rgb_alpha.
         */
        vdsi->textureObj[0].texPitch = texWidth * 4;
        vdsi->textureObj[0].dataSize = texWidth * texHeight * 4;
        vdsi->textureObj[0].data = pixels;
    }

    ASSERT(vdsi->textureObj[0].data != nullptr);

    VulkanPrepareTexture(
            &vc->gpu,
            &vc->device,
            &vc->setupCmd,
            &vc->cmdPool,
            &vc->queue,
            &vc->memoryProperties,
            useStagingBuffer,
            vdsi->textureObj,
            VK_IMAGE_LAYOUT_GENERAL);;

    stbi_image_free(pixels);
}
#endif
