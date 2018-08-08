static VkShaderModule
VulkanPrepareShaderModule(VkDevice *device, const void *code, size_t size)
{
    VkShaderModuleCreateInfo moduleCreateInfo;
    VkShaderModule module;
    VkResult err;

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = size;
    moduleCreateInfo.pCode = (const uint32_t *)code;

    err = vkCreateShaderModule(*device, &moduleCreateInfo, NULL, &module);
    ASSERT(!err);

    return module;
}

char *ReadSPV(const char *filename, size_t *psize)
{
    long int size;
    void *shaderCode;
    size_t retVal;

    FILE *fp = NULL;
    errno_t err = fopen_s(&fp, filename, "rb");
    if (err != 0)
    {
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    size = ftell(fp);

    fseek(fp, 0L, SEEK_SET);

    shaderCode = malloc(size);
    retVal = fread(shaderCode, size, 1, fp);
    if (!retVal)
    {
        return NULL;
    }

    *psize = size;

    fclose(fp);
    return (char *)shaderCode;
}

static VkShaderModule VulkanPrepareShader(
        VkDevice *device,
        const char * spvPath,
        VkShaderModule *shaderModule)
{
    void *shaderCode;
    size_t size;

    shaderCode = ReadSPV(spvPath, &size);

    *shaderModule = VulkanPrepareShaderModule(device, shaderCode, size);

    free(shaderCode);

    return *shaderModule;
}
