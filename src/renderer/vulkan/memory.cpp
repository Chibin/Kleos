/* Return the required amount of memory */
VkDeviceSize VulkanCreateBuffer(
        VkDevice *device,
        VkPhysicalDeviceMemoryProperties *memoryProperties,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer *buffer,
        VkDeviceMemory *bufferMemory)
{
    VkResult err;

    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext = NULL;
    bufferInfo.flags = 0;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.queueFamilyIndexCount = 0;
    bufferInfo.pQueueFamilyIndices = NULL;

    err = vkCreateBuffer(*device, &bufferInfo, nullptr, buffer);
    ASSERT(err == VK_SUCCESS);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*device, *buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = NULL;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = 0;
    b32 pass = AvailableMemoryTypeFromProperties(memoryProperties,
                                                 memRequirements.memoryTypeBits,
                                                 properties,
                                                 &allocInfo.memoryTypeIndex);
    ASSERT(pass);

    err = vkAllocateMemory(*device, &allocInfo, NULL, bufferMemory);
    ASSERT(err == VK_SUCCESS);

    err = vkBindBufferMemory(*device, *buffer, *bufferMemory, 0);
    ASSERT(err == VK_SUCCESS);

    return allocInfo.allocationSize;
}
