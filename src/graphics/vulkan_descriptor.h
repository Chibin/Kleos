void VulkanPrepareDescriptorSet(VulkanContext *vc)
{
    VkResult err;
    VkDevice *device = &vc->device;

    VkDescriptorImageInfo texDescs[DEMO_TEXTURE_COUNT] = {};

    VkDescriptorSetAllocateInfo allocInfo = {
        /*.sType =*/                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        /*.pNext =*/                NULL,
        /*.descriptorPool =*/       vc->descPool,
        /*.descriptorSetCount =*/   1,
        /*.pSetLayouts =*/          &vc->descLayout
    };
    err = vkAllocateDescriptorSets(*device, &allocInfo, &vc->descSet);
    ASSERT(!err);

    TextureObject *textures = vc->textures;
    for (memory_index i = 0; i < DEMO_TEXTURE_COUNT; i++)
    {
        texDescs[i].sampler = textures[i].sampler;
        texDescs[i].imageView = textures[i].view;
        texDescs[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    UniformObject *uniformData = &vc->uniformData;
    UniformObject *uniformDataFragment = &vc->uniformDataFragment;

    VkWriteDescriptorSet write;
    memset(&write, 0, sizeof(write));
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = vc->descSet;
    write.dstBinding = 0;
    write.descriptorCount = DEMO_TEXTURE_COUNT;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo = &uniformData->bufferInfo;
    write.pImageInfo = texDescs;
    vkUpdateDescriptorSets(*device, 1, &write, 0, NULL);

    VkWriteDescriptorSet writeFragment;
    memset(&writeFragment, 0, sizeof(writeFragment));
    writeFragment.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeFragment.dstSet = vc->descSet;
    writeFragment.dstBinding = 1;
    writeFragment.descriptorCount = DEMO_TEXTURE_COUNT;
    writeFragment.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeFragment.pBufferInfo = &uniformDataFragment->bufferInfo;
    writeFragment.pImageInfo = texDescs;
    vkUpdateDescriptorSets(*device, 1, &writeFragment, 0, NULL);
}
