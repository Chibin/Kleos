void VulkanSetDescriptorSet(
        VulkanContext *vc,
        VkDescriptorSet *descSet,
        TextureObject *textureObj,
        u32 textureObjCount,
        UniformObject *uniformData,
        UniformObject *uniformDataFragment)
{
    VkResult err;
    VkDevice *device = &vc->device;

    VkDescriptorImageInfo *texDescs =
        (VkDescriptorImageInfo *)malloc(sizeof(VkDescriptorImageInfo) * textureObjCount);
    memset(texDescs, 0, sizeof(VkDescriptorImageInfo) * textureObjCount);

    VkDescriptorSetAllocateInfo allocInfo = {
        /*.sType =*/                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        /*.pNext =*/                NULL,
        /*.descriptorPool =*/       vc->descPool,
        /*.descriptorSetCount =*/   1,
        /*.pSetLayouts =*/          &vc->descLayout
    };
    err = vkAllocateDescriptorSets(*device, &allocInfo, descSet);
    ASSERT(!err);

    for (memory_index i = 0; i < textureObjCount; i++)
    {
        texDescs[i].sampler = textureObj[i].sampler;
        texDescs[i].imageView = textureObj[i].view;
        texDescs[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    }

    VkWriteDescriptorSet write;
    memset(&write, 0, sizeof(write));
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = *descSet;
    write.dstBinding = 0;
    write.descriptorCount = textureObjCount;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.pBufferInfo = &uniformData->bufferInfo;
    write.pImageInfo = texDescs;
    vkUpdateDescriptorSets(*device, 1, &write, 0, NULL);

    VkWriteDescriptorSet writeFragment;
    memset(&writeFragment, 0, sizeof(writeFragment));
    writeFragment.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeFragment.dstSet = *descSet;
    writeFragment.dstBinding = 1;
    writeFragment.descriptorCount = textureObjCount;
    writeFragment.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeFragment.pBufferInfo = &uniformDataFragment->bufferInfo;
    writeFragment.pImageInfo = texDescs;
    vkUpdateDescriptorSets(*device, 1, &writeFragment, 0, NULL);

    free(texDescs);
}
