#ifndef __VULKAN__
#define __VULKAN__

#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>
#pragma warning(pop)

#define VK_USE_PLATFORM_WIN32_KHR
#define PLATFORM_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#define PlatformSurfaceCreateInfo VkWin32SurfaceCreateInfoKHR
#define PLATFORM_SURFACE_CREATE_INFO VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
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

struct VulkanBuffers
{
    VkBuffer bufs[100];
    VkDeviceMemory mems[100];
    memory_index maxNum;
    memory_index count;
};

struct VulkanVertices
{
    VkBuffer buf;
    VkDeviceMemory mem;

    VkPipelineVertexInputStateCreateInfo vi;
    VkVertexInputBindingDescription viBindings[1];
    VkVertexInputAttributeDescription viAttrs[3];
};

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

struct UniformObject
{
    VkBuffer buf;
    VkDeviceMemory mem;
    VkBufferCreateInfo bufInfo;
    VkDescriptorBufferInfo bufferInfo;
};

struct Depth
{
    VkFormat format;
    VkImage image;
    VkDeviceMemory mem;
    VkImageView view;
};

struct TextureObject
{
    VkSampler sampler;

    VkImage image;
    VkImageLayout imageLayout;

    VkDeviceMemory mem;
    VkImageView view;
    int32_t texWidth, texHeight;

    void *data;
    memory_index dataSize;
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
    VkRenderPass renderPassWithNoClear;
    VkPipeline pipeline;
    VkPipelineLayout pipelineLayout;
    VkSemaphore presentCompleteSemaphore;

    VkFormat format;

    VulkanVertices vertices;

    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;

    VkDescriptorPool descPool;
    VkDescriptorSet descSet;
    VkDescriptorSetLayout descLayout;

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

    UniformObject uniformData;
    UniformObject uniformDataFragment;
    Depth depth;
    TextureObject textures[DEMO_TEXTURE_COUNT];

    uint32_t swapchainImageCount;

    VkPhysicalDeviceMemoryProperties memoryProperties;
};

#endif
