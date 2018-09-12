/* create a pipeline barrier wtih the approriate src and dest stages
 */
#include "vulkan_barrier.h"

void VulkanCreateImage(
        VkDevice *device,
        VkPhysicalDeviceMemoryProperties *memoryProperties,
        u32 width,
        u32 height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage *image,
        VkDeviceMemory *imageMemory,
        VkImageLayout initialLayout
        )
{
    VkResult err = {};
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext = NULL;
    imageInfo.flags = 0;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent = {width, height, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = tiling;
    imageInfo.usage = usage;
    if (initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
    {
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
    imageInfo.initialLayout = initialLayout;;

    err = vkCreateImage(*device, &imageInfo, nullptr, image);
    ASSERT(err == VK_SUCCESS);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(*device, *image, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = 0;

    b32 pass = AvailableMemoryTypeFromProperties(
            memoryProperties,
            memRequirements.memoryTypeBits,
            properties,
            &allocInfo.memoryTypeIndex);
    ASSERT(pass);

    vkAllocateMemory(*device, &allocInfo, nullptr, imageMemory);
    ASSERT(err == VK_SUCCESS);

    err = vkBindImageMemory(*device, *image, *imageMemory, 0);
    ASSERT(!err);
}

static void VulkanSetTextureImage(
        VkDevice *device,
        VkCommandBuffer *cmdBuffer,
        VkCommandPool *cmdPool,
        VkPhysicalDeviceMemoryProperties *memoryProperties,
        struct TextureObject *texObj,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkFlags requiredProps)
{
            const VkFormat texFormat = VK_FORMAT_R8G8B8A8_UNORM;
            VkResult err;

            VulkanCreateImage(
                    device,
                    memoryProperties,
                    texObj->texWidth,
                    texObj->texHeight,
                    texFormat,
                    tiling,
                    usage,
                    requiredProps,
                    &texObj->image,
                    &texObj->mem,
                    VK_IMAGE_LAYOUT_PREINITIALIZED);

            if (requiredProps & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                const VkImageSubresource subres = {
                    /*.aspectMask =*/   VK_IMAGE_ASPECT_COLOR_BIT,
                    /*.mipLevel =*/     0,
                    /*.arrayLayer =*/   0,
                };

                VkSubresourceLayout layout;
                void *data;

                vkGetImageSubresourceLayout(*device, texObj->image, &subres, &layout);

                VkMemoryRequirements memReqs;
                vkGetImageMemoryRequirements(*device, texObj->image, &memReqs);
                err = vkMapMemory(*device, texObj->mem, 0, memReqs.size, 0, &data);
                ASSERT(!err);
                ASSERT(memReqs.size == texObj->dataSize);
                memcpy(data, texObj->data, memReqs.size);

                vkUnmapMemory(*device, texObj->mem);
            }

            /* setting the image layout does not reference the actual memory so no need
             * to add a mem ref
             */
            texObj->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            VulkanAddPipelineBarrier(
                    device,
                    cmdBuffer,
                    cmdPool,
                    texObj->image,
                    VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_PREINITIALIZED,
                    texObj->imageLayout,
                    VK_ACCESS_HOST_WRITE_BIT);
}

void VulkanCreateImageSampler(
        VkDevice *device,
        VkFormat texFormat,
        TextureObject *texture
        )
{
    VkResult err = {};
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext = NULL;
    samplerInfo.magFilter = VK_FILTER_NEAREST;
    samplerInfo.minFilter = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1;
    samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext = NULL;
    viewInfo.image = texture->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = texFormat;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R,
                            VK_COMPONENT_SWIZZLE_G,
                            VK_COMPONENT_SWIZZLE_B,
                            VK_COMPONENT_SWIZZLE_A,};
    viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    viewInfo.flags = 0;

    err = vkCreateSampler(*device, &samplerInfo, NULL, &texture->sampler);
    ASSERT(err == VK_SUCCESS);

    err = vkCreateImageView(*device, &viewInfo, NULL, &texture->view);
    ASSERT(err == VK_SUCCESS);
}

#if 0
void VulkanCreateTextureImage(
        VkDevice *device,
        VkPhysicalDeviceMemoryProperties *memoryProperties,
        const char* imagePath)
{
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(imagePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    ASSERT(!pixels);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    VulkanCreateBuffer(
            device,
            memoryProperties,
            imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &stagingBuffer,
            &stagingBufferMemory);

    void* data;
    vkMapMemory(*device, stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, (size_t)imageSize);
    vkUnmapMemory(*device, stagingBufferMemory);

    stbi_image_free(pixels);

    VulkanCreateImage(
            device,
            memoryProperties,
            texWidth,
            texHeight,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &textureImage,
            &textureImageMemory,
            VK_IMAGE_LAYOUT_UNDEFINED);
}
#endif
