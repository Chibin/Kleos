/* create a pipeline barrier wtih the approriate src and dest stages
 */
#include "./barrier.cpp"

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
    imageInfo.pNext = nullptr;
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
    allocInfo.pNext = nullptr;
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

void VulkanCopyImageFromHostToLocal(
        VkDevice *device,
        u32 pitch,
        u32 height,
        VkImage image,
        VkDeviceMemory mem,
        memory_index dataSize,
        u8 *data)
{
    VkResult err;
    const VkImageSubresource subres = {
        /*.aspectMask =*/   VK_IMAGE_ASPECT_COLOR_BIT,
        /*.mipLevel =*/     0,
        /*.arrayLayer =*/   0,
    };

    VkSubresourceLayout subResourceLayout;
    void *mapped;

    vkGetImageSubresourceLayout(*device, image, &subres, &subResourceLayout);

    VkMemoryRequirements memReqs;
    vkGetImageMemoryRequirements(*device, image, &memReqs);
    err = vkMapMemory(*device, mem, 0, memReqs.size, 0, &mapped);
    ASSERT(!err);

    /* Let's just assume that the data is perfectly aligned if they
     * are the same size.
     */
    if (memReqs.size == dataSize)
    {
        memcpy(mapped, data, memReqs.size);
    }
    else
    {
        VkDeviceSize counter = 0;
        mapped = (u8 *)mapped + subResourceLayout.offset;

        VkDeviceSize bytesToCopy = pitch;
        ASSERT(bytesToCopy > 0);
        VkDeviceSize remaining = subResourceLayout.rowPitch - bytesToCopy;

        /* When the image has an odd amount of width/height, then
         * the SDL_Surface will most likely have an misaligned
         * pitch vs the subResource layout. I'm realigning it here
         * to adhere to Vulkan requirements. Otherwise, you'll get
         * jumbled sample image.
         */
        for (memory_index y = 0; y < height; y++)
        {
            memcpy(mapped, data, bytesToCopy);
            /* If the width of the new bitmap we are trying to copy is less than the previous one,
             * then it's possible to have left over data from the previous bitmap image.
             * This will clear it out.
             */
            if (remaining > 0)
            {
                memset((u8 *)mapped + bytesToCopy, 0, remaining);
            }

            mapped = (u8 *)mapped + subResourceLayout.rowPitch;
            data += bytesToCopy;
            counter += subResourceLayout.rowPitch;
        }
        ASSERT(counter == memReqs.size);
    }

    vkUnmapMemory(*device, mem);
}

static void VulkanSetTextureImage(
        VkDevice *device,
        VkCommandBuffer *cmdBuffer,
        VkCommandPool *cmdPool,
        VkPhysicalDeviceMemoryProperties *memoryProperties,
        struct TextureObject *texObj,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkFlags requiredProps,
        VkFormat texFormat,
        VkImageLayout desiredLayout)
{
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
                VulkanCopyImageFromHostToLocal(
                        device,
                        texObj->texPitch,
                        texObj->texHeight,
                        texObj->image,
                        texObj->mem,
                        texObj->dataSize,
                        (u8 *)texObj->data);
            }

            /* setting the image layout does not reference the actual memory so no need
             * to add a mem ref
             */
            //texObj->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            texObj->imageLayout = desiredLayout;
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
    samplerInfo.pNext = nullptr;
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
    viewInfo.pNext = nullptr;
    viewInfo.image = texture->image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = texFormat;
    viewInfo.components = { VK_COMPONENT_SWIZZLE_R,
                            VK_COMPONENT_SWIZZLE_G,
                            VK_COMPONENT_SWIZZLE_B,
                            VK_COMPONENT_SWIZZLE_A,};
    viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    viewInfo.flags = 0;

    err = vkCreateSampler(*device, &samplerInfo, nullptr, &texture->sampler);
    ASSERT(err == VK_SUCCESS);

    err = vkCreateImageView(*device, &viewInfo, nullptr, &texture->view);
    ASSERT(err == VK_SUCCESS);
}

void VulkanUseStagingBufferToCopyLinearTextureToOptimized(
        VkDevice *device,
        VkCommandBuffer *setupCmd,
        VkCommandPool *cmdPool,
        VkQueue *queue,
        VkPhysicalDeviceMemoryProperties *memoryProperties,
        TextureObject *texture
        )
{
    struct TextureObject stagingTexture;

    memset(&stagingTexture, 0, sizeof(stagingTexture));
    VulkanSetTextureImage(
            device,
            setupCmd,
            cmdPool,
            memoryProperties,
            &stagingTexture,
            VK_IMAGE_TILING_LINEAR,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VulkanSetTextureImage(
            device,
            setupCmd,
            cmdPool,
            memoryProperties,
            texture,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    VulkanAddPipelineBarrier(
            device,
            setupCmd,
            cmdPool,
            stagingTexture.image,
            VK_IMAGE_ASPECT_COLOR_BIT,
            stagingTexture.imageLayout,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            (VkAccessFlagBits)0);

    VulkanAddPipelineBarrier(
            device,
            setupCmd,
            cmdPool,
            texture->image,
            VK_IMAGE_ASPECT_COLOR_BIT,
            texture->imageLayout,
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
            *setupCmd, stagingTexture.image,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, texture->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    VulkanAddPipelineBarrier(
            device,
            setupCmd,
            cmdPool,
            texture->image,
            VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            texture->imageLayout,
            (VkAccessFlagBits)0);

    VulkanFlushInit(device, setupCmd, cmdPool, queue);
    VulkanDestroyTextureImage(device, &stagingTexture);
}

void VulkanPrepareTexture(
        VkPhysicalDevice *gpu,
        VkDevice *device,
        VkCommandBuffer *setupCmd,
        VkCommandPool *cmdPool,
        VkQueue *queue,
        VkPhysicalDeviceMemoryProperties *memoryProperties,
        bool useStagingBuffer,
        TextureObject *textures,
        VkImageLayout desiredLayout)
{
    const VkFormat texFormat = VK_FORMAT_R8G8B8A8_UNORM;
    VkFormatProperties props = {};

    vkGetPhysicalDeviceFormatProperties(*gpu, texFormat, &props);

    for (memory_index i = 0; i < DEMO_TEXTURE_COUNT; i++)
    {
        if ((props.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) && !useStagingBuffer)
        {
            /* Device can texture using linear textures */
            ASSERT(textures[i].texWidth > 0);
            ASSERT(textures[i].texHeight > 0);
            ASSERT(textures[i].dataSize > 0);
            ASSERT(textures[i].data != nullptr);

            VulkanSetTextureImage(
                    device,
                    setupCmd,
                    cmdPool,
                    memoryProperties,
                    &textures[i],
                    VK_IMAGE_TILING_LINEAR,
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                    texFormat,
                    desiredLayout);

        }
        else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
        {

            VulkanUseStagingBufferToCopyLinearTextureToOptimized(
                    device,
                    setupCmd,
                    cmdPool,
                    queue,
                    memoryProperties,
                    &textures[i]
                    );

        }
        else
        {
            /* Can't support VK_FORMAT_R8G8B8A8_UNORM !? */
            ASSERT(!"No support for B8G8R8A8_UNORM as texture image format");
        }

        VulkanCreateImageSampler(
                device,
                texFormat,
                &textures[i]);
    }

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
