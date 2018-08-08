/* set and create descriptor layouts
 * define push constant header
 * create pipeline layout
 */
void VulkanInitPipelineLayout(VulkanContext *vc)
{
    VkResult err;
    VkDevice *device = &vc->device;

    VkDescriptorSetLayoutBinding uboBinding = {};
    uboBinding.binding = 0;
    uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboBinding.descriptorCount = 1;
    uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboBinding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    samplerLayoutBinding.pImmutableSamplers = NULL;

    const VkDescriptorSetLayoutBinding myDescriptorSetLayoutBinding[] = {uboBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
    descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayout.pNext = NULL;
    descriptorLayout.bindingCount = 2;
    descriptorLayout.pBindings = &myDescriptorSetLayoutBinding[0];

    err = vkCreateDescriptorSetLayout(*device, &descriptorLayout, NULL, &vc->descLayout);
    ASSERT(!err);

    VkPushConstantRange pushConstantRange;
    memset(&pushConstantRange, 0, sizeof(VkPushConstantRange));
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    /* view and projection matrices */
    pushConstantRange.size = sizeof(glm::mat4) * 2;

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = NULL;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &vc->descLayout;
    pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
    pPipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

    err = vkCreatePipelineLayout(*device, &pPipelineLayoutCreateInfo, NULL, &vc->pipelineLayout);
    ASSERT(!err);

}

