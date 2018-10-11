void VulkanInitRenderPass(VulkanContext *vc)
{
    VkResult err;
    /* attachments are ordered based on how we create the frame buffer?*/
    const VkAttachmentDescription attachments[ATTACHMENT_COUNT] = {
        {
            /*.flags =*/            VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT,
            /*.format =*/           vc->format,
            /*.samples =*/          VK_SAMPLE_COUNT_1_BIT,
            /*.loadOp =*/           VK_ATTACHMENT_LOAD_OP_CLEAR,
            /*.storeOp =*/          VK_ATTACHMENT_STORE_OP_STORE,
            /*.stencilLoadOp =*/    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            /*.stencilStoreOp =*/   VK_ATTACHMENT_STORE_OP_DONT_CARE,
            /*.initialLayout =*/    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            /*.finalLayout =*/      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        },
        {
            /*.flags =*/            0,
            /*.format =*/           vc->depth.format,
            /*.samples =*/          VK_SAMPLE_COUNT_1_BIT,
            /*.loadOp =*/           VK_ATTACHMENT_LOAD_OP_CLEAR,
            /*.storeOp =*/          VK_ATTACHMENT_STORE_OP_DONT_CARE,
            /*.stencilLoadOp =*/    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            /*.stencilStoreOp =*/   VK_ATTACHMENT_STORE_OP_DONT_CARE,
            /*.initialLayout =*/    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            /*.finalLayout =*/      VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        },
        {
            /*.flags =*/            VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT,
            /*.format =*/           vc->format,
            /*.samples =*/          VK_SAMPLE_COUNT_1_BIT,
            /*.loadOp =*/           VK_ATTACHMENT_LOAD_OP_LOAD,
            /*.storeOp =*/          VK_ATTACHMENT_STORE_OP_STORE,
            /*.stencilLoadOp =*/    VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            /*.stencilStoreOp =*/   VK_ATTACHMENT_STORE_OP_DONT_CARE,
            /*.initialLayout =*/    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            /*.finalLayout =*/      VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        },
    };

    const VkAttachmentReference colorReference = {
        /*.attachment =*/   0,
        /*.layout =*/       VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };

    const VkAttachmentReference depthReference = {
        /*.attachment =*/   1,
        /*.layout =*/       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    const VkAttachmentReference noDepthReference = {
        /*.attachment =*/   0,
        /*.layout =*/       VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    const VkSubpassDescription depthStencilSubPass = {
        /*.flags =*/                    0,
        /*.pipelineBindPoint =*/        VK_PIPELINE_BIND_POINT_GRAPHICS,
        /*.inputAttachmentCount =*/     0,
        /*.pInputAttachments =*/        nullptr,
        /*.colorAttachmentCount =*/     1,
        /*.pColorAttachments =*/        &colorReference,
        /*.pResolveAttachments =*/      nullptr,
        /*.pDepthStencilAttachment =*/  &depthReference,
        /*.preserveAttachmentCount =*/  0,
        /*.pPreserveAttachments =*/     nullptr,
    };

    const VkSubpassDescription UISubPass = {
        /*.flags =*/                    0,
        /*.pipelineBindPoint =*/        VK_PIPELINE_BIND_POINT_GRAPHICS,
        /*.inputAttachmentCount =*/     0,
        /*.pInputAttachments =*/        nullptr,
        /*.colorAttachmentCount =*/     1,
        /*.pColorAttachments =*/        &colorReference,
        /*.pResolveAttachments =*/      nullptr,
        /*.pDepthStencilAttachment =*/  0,
        /*.preserveAttachmentCount =*/  0,
        /*.pPreserveAttachments =*/     nullptr,
    };

    const VkSubpassDescription subPasses[] = {depthStencilSubPass, UISubPass};

    VkSubpassDependency subpassDependency = {
        /*uint32_t                srcSubpass*/      0,
        /*uint32_t                dstSubpass*/      1,
        /*VkPipelineStageFlags    srcStageMask*/    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        /*VkPipelineStageFlags    dstStageMask*/    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        /*VkAccessFlags           srcAccessMask*/   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        /*VkAccessFlags           dstAccessMask*/   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
        /*VkDependencyFlags       dependencyFlags*/ VK_DEPENDENCY_BY_REGION_BIT,
    };

    const VkRenderPassCreateInfo rpInfo = {
        /*.sType =*/            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        /*.pNext =*/            nullptr,
        /*.flags =*/            0,
        /*.attachmentCount =*/  ATTACHMENT_COUNT,
        /*.pAttachments =*/     attachments,
        /*.subpassCount =*/     2,
        /*.pSubpasses =*/       subPasses,
        /*.dependencyCount =*/  1,
        /*.pDependencies =*/    &subpassDependency,
    };

    err = vkCreateRenderPass(vc->device, &rpInfo, nullptr, &vc->renderPass);
    ASSERT(!err);
}
