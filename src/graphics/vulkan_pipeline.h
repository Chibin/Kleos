void VulkanPreparePipeline(VulkanContext *vc, u32 stride)
{
    VkResult err;
    VkShaderModule vertShaderModule = {};
    VkShaderModule fragShaderModule = {};

    vc->vertices.viBindings[0].binding = VERTEX_BUFFER_BIND_ID;
    vc->vertices.viBindings[0].stride = stride;
    vc->vertices.viBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    /* the attr order is based on the shader code  */
    vc->vertices.viAttrs[0].binding = VERTEX_BUFFER_BIND_ID;
    vc->vertices.viAttrs[0].location = 0;
    vc->vertices.viAttrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vc->vertices.viAttrs[0].offset = offsetof(Vertex, position);

    vc->vertices.viAttrs[1].binding = VERTEX_BUFFER_BIND_ID;
    vc->vertices.viAttrs[1].location = 1;
    vc->vertices.viAttrs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vc->vertices.viAttrs[1].offset = offsetof(Vertex, color);

    vc->vertices.viAttrs[2].binding = VERTEX_BUFFER_BIND_ID;
    vc->vertices.viAttrs[2].location = 2;
    vc->vertices.viAttrs[2].format = VK_FORMAT_R32G32_SFLOAT;
    vc->vertices.viAttrs[2].offset = offsetof(Vertex, uv);

    VkPipelineVertexInputStateCreateInfo vi;
    vc->vertices.vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vc->vertices.vi.pNext = NULL;
    vc->vertices.vi.vertexBindingDescriptionCount = 1;
    vc->vertices.vi.pVertexBindingDescriptions = vc->vertices.viBindings;
    vc->vertices.vi.vertexAttributeDescriptionCount = 3;
    vc->vertices.vi.pVertexAttributeDescriptions = vc->vertices.viAttrs;
    vi = vc->vertices.vi;

    VkPipelineInputAssemblyStateCreateInfo ia = {};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rs = {};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.depthClampEnable = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.depthBiasEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo vp = {};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;

    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE] = {};
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables;

    /* If the wide lines feature is not enabled, and no element of the
     * pDynamicStates member of pDynamicState is VK_DYNAMIC_STATE_LINE_WIDTH,
     * then lineWidth member of pRasterizationState must be 1.0
     */
    if (*dynamicState.pDynamicStates != VK_DYNAMIC_STATE_LINE_WIDTH)
    {
        rs.lineWidth = 1.0;
    }

    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineColorBlendAttachmentState attState[1] = {};
    attState[0].colorWriteMask = 0xf;
    attState[0].blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo cb = {};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = attState;

    VkPipelineDepthStencilStateCreateInfo ds = {};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.back.failOp = VK_STENCIL_OP_KEEP;
    ds.back.passOp = VK_STENCIL_OP_KEEP;
    ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
    ds.stencilTestEnable = VK_FALSE;
    ds.front = ds.back;

    VkPipelineMultisampleStateCreateInfo ms = {};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.pSampleMask = NULL;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Two stages: vs and fs
    VkPipelineShaderStageCreateInfo shaderStages[2] = {};

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = VulkanPrepareShader(&vc->device,
            "./materials/programs/vulkan/test-vert.spv",
            &vertShaderModule);
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = VulkanPrepareShader(&vc->device,
           "./materials/programs/vulkan/test-frag.spv",
           &fragShaderModule);
    shaderStages[1].pName = "main";

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    /*
    pipelineCreateInfo.pNext;
    pipelineCreateInfo.flags;
    */
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;

    pipelineCreateInfo.pVertexInputState = &vi;
    pipelineCreateInfo.pInputAssemblyState = &ia;
    /* pipelineCreateInfo.pTessellationState; */
    pipelineCreateInfo.pViewportState = &vp;
    pipelineCreateInfo.pRasterizationState = &rs;
    pipelineCreateInfo.pMultisampleState = &ms;
    pipelineCreateInfo.pDepthStencilState = &ds;
    pipelineCreateInfo.pColorBlendState = &cb;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.layout = vc->pipelineLayout;
    pipelineCreateInfo.renderPass = vc->renderPass;
    pipelineCreateInfo.subpass = 0;
    /*
    pipelineCreateInfo.basePipelineHandle;
    pipelineCreateInfo.basePipelineIndex;
    */

    VkPipelineCache pipelineCache = {};
    VkPipelineCacheCreateInfo pipelineCacheInfo = {};
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    err = vkCreatePipelineCache(vc->device, &pipelineCacheInfo, NULL, &pipelineCache);
    ASSERT(!err);

    err = vkCreateGraphicsPipelines(vc->device, pipelineCache, 1, &pipelineCreateInfo, NULL, &vc->pipeline);
    ASSERT(!err);

    vkDestroyPipelineCache(vc->device, pipelineCache, NULL);

    vkDestroyShaderModule(vc->device, fragShaderModule, NULL);
    vkDestroyShaderModule(vc->device, vertShaderModule, NULL);

}

void VulkanSetVertexAttributes(VulkanContext *vc)
{
    /* the attr order is based on the shader code  */
    vc->vertices.viAttrs[0].binding = VERTEX_BUFFER_BIND_ID;
    vc->vertices.viAttrs[0].location = 0;
    vc->vertices.viAttrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vc->vertices.viAttrs[0].offset = offsetof(Vertex, position);

    vc->vertices.viAttrs[1].binding = VERTEX_BUFFER_BIND_ID;
    vc->vertices.viAttrs[1].location = 1;
    vc->vertices.viAttrs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vc->vertices.viAttrs[1].offset = offsetof(Vertex, color);

    vc->vertices.viAttrs[2].binding = VERTEX_BUFFER_BIND_ID;
    vc->vertices.viAttrs[2].location = 2;
    vc->vertices.viAttrs[2].format = VK_FORMAT_R32G32_SFLOAT;
    vc->vertices.viAttrs[2].offset = offsetof(Vertex, uv);
}

void VulkanSetVertexBindings(VulkanContext *vc, u32 stride)
{
    vc->vertices.viBindings[0].binding = VERTEX_BUFFER_BIND_ID;
    vc->vertices.viBindings[0].stride = stride;
    vc->vertices.viBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
}

void VulkanPrepare2ndPipeline(VulkanContext *vc, u32 stride)
{
    VkResult err;
    VkShaderModule vertShaderModule = {};
    VkShaderModule fragShaderModule = {};

    VulkanSetVertexBindings(vc, stride);
    VulkanSetVertexAttributes(vc);

    VkPipelineVertexInputStateCreateInfo vi;
    vc->vertices.vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vc->vertices.vi.pNext = NULL;
    vc->vertices.vi.vertexBindingDescriptionCount = 1;
    vc->vertices.vi.pVertexBindingDescriptions = vc->vertices.viBindings;
    vc->vertices.vi.vertexAttributeDescriptionCount = 3;
    vc->vertices.vi.pVertexAttributeDescriptions = vc->vertices.viAttrs;
    vi = vc->vertices.vi;

    VkPipelineInputAssemblyStateCreateInfo ia = {};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rs = {};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.depthClampEnable = VK_FALSE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.depthBiasEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo vp = {};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1;
    vp.scissorCount = 1;

    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE] = {};
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables;

    /* If the wide lines feature is not enabled, and no element of the
     * pDynamicStates member of pDynamicState is VK_DYNAMIC_STATE_LINE_WIDTH,
     * then lineWidth member of pRasterizationState must be 1.0
     */
    if (*dynamicState.pDynamicStates != VK_DYNAMIC_STATE_LINE_WIDTH)
    {
        rs.lineWidth = 1.0;
    }

    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineColorBlendAttachmentState attState[1] = {};
    attState[0].colorWriteMask = 0xf;
    attState[0].blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo cb = {};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = attState;

    VkPipelineDepthStencilStateCreateInfo ds = {};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.back.failOp = VK_STENCIL_OP_KEEP;
    ds.back.passOp = VK_STENCIL_OP_KEEP;
    ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
    ds.stencilTestEnable = VK_FALSE;
    ds.front = ds.back;

    VkPipelineMultisampleStateCreateInfo ms = {};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.pSampleMask = NULL;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Two stages: vs and fs
    VkPipelineShaderStageCreateInfo shaderStages[2] = {};

    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = VulkanPrepareShader(&vc->device,
            "./materials/programs/vulkan/screen_space-vert.spv",
            &vertShaderModule);
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = VulkanPrepareShader(&vc->device,
           "./materials/programs/vulkan/test-frag.spv",
           &fragShaderModule);
    shaderStages[1].pName = "main";

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    /*
    pipelineCreateInfo.pNext;
    pipelineCreateInfo.flags;
    */
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;

    pipelineCreateInfo.pVertexInputState = &vi;
    pipelineCreateInfo.pInputAssemblyState = &ia;
    /* pipelineCreateInfo.pTessellationState; */
    pipelineCreateInfo.pViewportState = &vp;
    pipelineCreateInfo.pRasterizationState = &rs;
    pipelineCreateInfo.pMultisampleState = &ms;
    pipelineCreateInfo.pDepthStencilState = &ds;
    pipelineCreateInfo.pColorBlendState = &cb;
    pipelineCreateInfo.pDynamicState = &dynamicState;
    pipelineCreateInfo.layout = vc->pipelineLayout;
    pipelineCreateInfo.renderPass = vc->renderPass;
    pipelineCreateInfo.subpass = 1;
    /*
    pipelineCreateInfo.basePipelineHandle;
    pipelineCreateInfo.basePipelineIndex;
    */

    VkPipelineCache pipelineCache = {};
    VkPipelineCacheCreateInfo pipelineCacheInfo = {};
    pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    err = vkCreatePipelineCache(vc->device, &pipelineCacheInfo, NULL, &pipelineCache);
    ASSERT(!err);

    err = vkCreateGraphicsPipelines(vc->device, pipelineCache, 1, &pipelineCreateInfo, NULL, &vc->pipeline2);
    ASSERT(!err);

    vkDestroyPipelineCache(vc->device, pipelineCache, NULL);

    vkDestroyShaderModule(vc->device, fragShaderModule, NULL);
    vkDestroyShaderModule(vc->device, vertShaderModule, NULL);

}
