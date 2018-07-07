#ifndef __VULKAN__
#define __VULKAN__

#define VK_USE_PLATFORM_WIN32_KHR
#define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#define PlatformSurfaceCreateInfo VkWin32SurfaceCreateInfoKHR
#define PLATFORM_SURFACE_CREATE_INFO VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
#define PLATFORM_ASSIGN_SURFACEINFO \
    createInfo.hinstance = GetModuleHandle(NULL); \
    createInfo.hwnd = info.info.win.window
#define PlatformCreateSurface vkCreateWin32SurfaceKHR
#define DEMO_TEXTURE_COUNT 1
#define VERTEX_BUFFER_BIND_ID 0

#if WIN32
#include "SDL_syswm.h"
#endif

#include <vulkan/vulkan.h>

typedef struct _SwapchainBuffers
{
    VkImage image;
    VkCommandBuffer cmd;
    VkImageView view;
} SwapchainBuffers;

struct VulkanVertices
{
    VkBuffer buf;
    VkDeviceMemory mem;

    VkPipelineVertexInputStateCreateInfo vi;
    VkVertexInputBindingDescription viBindings[1];
    VkVertexInputAttributeDescription viAttrs[2];
};

struct VulkanContext
{
    VkDevice device;
    VkSurfaceKHR surface;
    VkPhysicalDevice gpu;

    VkSwapchainKHR swapchain;
    SwapchainBuffers *buffers;

    VkFramebuffer *framebuffers;

    VkQueue queue;
    VkCommandBuffer setupCmd;
    VkCommandBuffer drawCmd;  // Command Buffer for drawing commands
    VkCommandPool cmdPool;

    VkRenderPass renderPass;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;

    VulkanVertices vertices;

    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

    VkDescriptorPool descPool;
    VkDescriptorSet descSet;

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

    uint32_t currentBuffer;

    int32_t curFrame;
	int32_t frameCount;
    uint32_t width;
    uint32_t height;

    float depthStencil;
    float depthIncrement;

    b32 quit;
};

struct TextureObject
{
    VkSampler sampler;

    VkImage image;
    VkImageLayout imageLayout;

    VkDeviceMemory mem;
    VkImageView view;
    int32_t texWidth, texHeight;
};

#endif
