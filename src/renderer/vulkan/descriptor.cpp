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
        /*.pNext =*/                nullptr,
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
    vkUpdateDescriptorSets(*device, 1, &write, 0, nullptr);

    VkWriteDescriptorSet writeFragment;
    memset(&writeFragment, 0, sizeof(writeFragment));
    writeFragment.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeFragment.dstSet = *descSet;
    writeFragment.dstBinding = 1;
    writeFragment.descriptorCount = textureObjCount;
    writeFragment.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeFragment.pBufferInfo = &uniformDataFragment->bufferInfo;
    writeFragment.pImageInfo = texDescs;
    vkUpdateDescriptorSets(*device, 1, &writeFragment, 0, nullptr);

    free(texDescs);
}

void VulkanSetDescriptorSet(VulkanContext *vc, VulkanDescriptorSetInfo *vdsi)
{
    VulkanSetDescriptorSet(
            vc,
            &vdsi->descSet,
            vdsi->textureObj,
            vdsi->textureObjCount,
            vdsi->uniformData,
            vdsi->uniformDataFragment);
}

void VulkanPrepareDescriptorPool(VulkanContext *vc)
{
    /* TODO: Update parameters to account of max sets and poolsize count */

    VkResult err;
    /* does not need to be "in order" */
    const VkDescriptorPoolSize typeCount[] = {
        {
        /*.type =*/             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        /*.descriptorCount =*/  DEMO_TEXTURE_COUNT,
        },
        {
        /*.type =*/             VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        /*.descriptorCount =*/  DEMO_TEXTURE_COUNT,
        },
    };

    const VkDescriptorPoolCreateInfo descriptorPool = {
        /*.sType =*/            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        /*.pNext =*/            nullptr,
        /*.flags =*/            0,
        /* Max set is talking about the amount of descriptor sets we can
         * allocate? Each descriptor set we allocate will have the
         * type count? So, 1 uniform buffer, and 1 sampler.
         */
        /*.maxSets =*/          7,
        /*.poolSizeCount =*/    2,
        /*.pPoolSizes =*/       &typeCount[0],
    };

    err = vkCreateDescriptorPool(vc->device, &descriptorPool, nullptr, &vc->descPool);
    ASSERT(!err);
}

