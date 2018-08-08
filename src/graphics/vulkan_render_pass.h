void VulkanInitRenderPass(VulkanContext *vc)
{
    VkResult err;
    /* attachments are ordered based on how we create the frame buffer?*/
    const VkAttachmentDescription attachments[ATTACHMENT_COUNT] = {
        {
            /*.flags =*/            0,
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
        /*.pInputAttachments =*/        NULL,
        /*.colorAttachmentCount =*/     1,
        /*.pColorAttachments =*/        &colorReference,
        /*.pResolveAttachments =*/      NULL,
        /*.pDepthStencilAttachment =*/  &depthReference,
        /*.preserveAttachmentCount =*/  0,
        /*.pPreserveAttachments =*/     NULL,
    };

    const VkSubpassDescription UISubPass = {
        /*.flags =*/                    0,
        /*.pipelineBindPoint =*/        VK_PIPELINE_BIND_POINT_GRAPHICS,
        /*.inputAttachmentCount =*/     0,
        /*.pInputAttachments =*/        NULL,
        /*.colorAttachmentCount =*/     1,
        /*.pColorAttachments =*/        &colorReference,
        /*.pResolveAttachments =*/      NULL,
        /*.pDepthStencilAttachment =*/  &depthReference,
        /*.preserveAttachmentCount =*/  0,
        /*.pPreserveAttachments =*/     NULL,
    };

    const VkSubpassDescription subPasses[] = {UISubPass, depthStencilSubPass};

    const VkRenderPassCreateInfo rpInfo = {
        /*.sType =*/            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        /*.pNext =*/            NULL,
        /*.flags =*/            0,
        /*.attachmentCount =*/  ATTACHMENT_COUNT,
        /*.pAttachments =*/     attachments,
        /*.subpassCount =*/     1,
        /*.pSubpasses =*/       &depthStencilSubPass,
        /*.dependencyCount =*/  0,
        /*.pDependencies =*/    NULL,
    };

    err = vkCreateRenderPass(vc->device, &rpInfo, NULL, &vc->renderPass);
    ASSERT(!err);
}
