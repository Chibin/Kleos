void HandleInput(GameMetadata *gameMetadata, b32 *continueRunning)
{
    SDL_Event event;

    /* NOTE: Looks very player centric right now, not sure if we need to make it
     * better later on.
     * Use keystate to get the feel for the fastest response time.
     * Keyrepeat is doesn't feel smooth enough for us to use it as a way to
     * move. The amount of keyboard repeat depends on the setting the user has
     * on their computer, so not reliable.
     */
    const u8 *keystate = SDL_GetKeyboardState(nullptr);
    if (gameMetadata->isEditMode == false)
    {
        ProcessKeysHeldDown(g_player, keystate);
    }
    else if(gameMetadata->isEditMode && gameMetadata->selectedRect != nullptr)
    {
        ProcessKeysHeldDownEditMode(gameMetadata, keystate);
    }

    while (SDL_PollEvent(&event) != 0)
    {
        switch (event.type)
        {
        case SDL_QUIT:
            *continueRunning = false;
            break;
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            ProcessMouseEditMode(gameMetadata, g_camera, g_projection, event);
        case SDL_MOUSEWHEEL:
            ProcessMouseInput(event, g_camera);
            break;
        case SDL_MOUSEMOTION:
            g_mousePoint = ProcessMouseMotion(event.motion);
            UpdateMouseDrag(gameMetadata, g_camera, g_projection, event);
            break;
        case SDL_KEYDOWN:
            ProcessInputDown(
                    event.key.keysym.sym,
                    gameMetadata,
                    g_camera,
                    continueRunning);
            break;
        case SDL_KEYUP:
            ProcessInputUp(event.key.keysym.sym);
            break;
        default:
            break;
        }
    }
}

void SetFont(GameMetadata *gm)
{
    TTF_Font *font = OpenFont();
    ASSERT(font != nullptr);
    gm->font = font;
    //ASSERT(TTF_FontFaceIsFixedWidth(font) == 1);
}

void SetCamera(GameMetadata *gm)
{
        v3 cameraPos = { 0, 0, 5 };
        // and looks at the origin
        v3 cameraTarget = { 0, 0, 0 };
        // Head is up (set to 0,1,0 to look upside-down)
        v3 cameraUp = { 0, 1, 0 };
        g_camera = CreateCamera(&gm->reservedMemory, cameraPos, cameraTarget, cameraUp);
}

void SetGameTimeStep(GameMetadata *gm)
{
    GameTimestep **gameTimestep = &gm->gameTimestep;
    *gameTimestep = (GameTimestep *)AllocateMemory(&gm->reservedMemory, sizeof(GameTimestep));
    ResetGameTimestep(*gameTimestep);
}

void SetPerspectiveProjection(GameMetadata *gm)
{
    f32 screenWidth = gm->screenResolution.v[0];
    f32 screenHeight = gm->screenResolution.v[1];
    g_projection = (glm::mat4 *)AllocateMemory(&gm->reservedMemory, (sizeof(glm::mat4)));
    *g_projection =
        //glm::perspective(glm::radians(45.0f), screenWidth / screenHeight, 0.1f, 1000.0f);
        //glm::infinitePerspective(45.0f, screenWidth / screenHeight, 1.0f);
        glm::mat4(PerspectiveProjectionMatrix(Radians(45.0f), screenWidth / screenHeight, 0.1f, 1000.0f));
}

void SetPreparePipeline(VulkanContext *vc)
{
    memset(&vc->vertices, 0, sizeof(vc->vertices));
    VulkanPreparePipeline(vc, sizeof(Vertex));
    VulkanPrepare2ndPipeline(vc, sizeof(Vertex));
}

void SetHash(GameMetadata *gm)
{
    HASH_CREATE(HashBitmapBitmap, &gm->reservedMemory, hashBitmap, MAX_HASH);
    HASH_CREATE(HashBitmapVkDescriptorSet, &gm->reservedMemory, hashBMPDescSet, MAX_HASH);
    gm->hashBitmapVkDescriptorSet = hashBMPDescSet;
    gm->hashBitmap = hashBitmap;
}

void SetWhiteBitmap(GameMetadata *gm)
{
    snprintf(gm->whiteBitmap.name, sizeof(gm->whiteBitmap.name), "%s", "white");
    gm->whiteBitmap.width = 1;
    gm->whiteBitmap.height = 1;
    gm->whiteBitmap.format = GL_RGBA;
    gm->whiteBitmap.bitmapID = MAX_HASH - 1; // XXX: needs to be changed somewhere
    gm->whiteBitmap.textureParam = TextureParam{ GL_NEAREST, GL_NEAREST };
    gm->whiteBitmap.data =
        (u8 *)AllocateMemory(&gm->reservedMemory, gm->whiteBitmap.width * gm->whiteBitmap.height * sizeof(u32));
    for (memory_index i = 0; i < gm->whiteBitmap.width * gm->whiteBitmap.height; i++)
    {
        /* alpha -> blue -> green -> red: 1 byte each */
#if 0
        *((u32 *)gm->whiteBitmap.data + i) = 0x33333333;
#else
        *((u32 *)gm->whiteBitmap.data + i) = 0xFF222222;
#endif
    }
}

void SetFontBitmap(GameMetadata *gm)
{
    char buffer[256];
    u8 capitalLetters[27] = {};
    u8 lowercaseLetters[27] = {};
    u8 specialCharacters[6] = {'/', '.', ',', '!', '-', };
    u8 numbers[11] = {}; /* extra byte for '\n' */

    for(memory_index i = 0; i < 26; i++)
    {
        capitalLetters[i] = 'A' + SafeCastToU32(i);
        lowercaseLetters[i] = 'a' + SafeCastToU32(i);
    }

    for(memory_index i = 0; i < 10; i++)
    {
        numbers[i] = '0' + SafeCastToU32(i);
    }
    sprintf_s(buffer, sizeof(char) * 150, "%s%s%s%s",
            numbers, capitalLetters, lowercaseLetters, specialCharacters);

    StringToBitmap(&gm->fontBitmap, gm->font, buffer);                                // NOLINT

    snprintf(gm->fontBitmap.name, sizeof(gm->fontBitmap.name), "%s", "font");
    gm->fontBitmap.bitmapID = MAX_HASH - 3; // XXX: needs to be changed somewhere
    gm->fontBitmap.textureParam = TextureParam{ GL_NEAREST, GL_NEAREST };
    ASSERT(gm->fontBitmap.data != nullptr);
}


void SetBitmapToGPUForPipeline(VulkanContext *vc, GameMetadata *gameMetadata, Bitmap *stringBitmap)
{
    SetWhiteBitmap(gameMetadata);
    SetFontBitmap(gameMetadata);

    bool useStagingBuffer = false;
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;
    stringBitmap->bitmapID = MAX_HASH - 2; // Arbitrary number
    char buffer[256];
    /* Hack: There's a bunch of blank spaces at the end to accomodate
     * the amout of extra characters for later images.
     * This will give us a longer width when creating a vkimage.
     */

    sprintf_s(buffer, sizeof(char) * 150, "   %.02f ms/f    %.0ff/s    %.02fcycles/f              ", 33.0f, 66.0f, 99.0f); // NOLINT
    StringToBitmap(stringBitmap, gameMetadata->font, buffer);                                    // NOLINT
    stringBitmap->textureParam = TextureParam{ GL_NEAREST,  GL_NEAREST };

    vc->UITextures[0].texWidth = stringBitmap->width;
    vc->UITextures[0].texHeight = stringBitmap->height;
    vc->UITextures[0].texPitch = stringBitmap->pitch;
    vc->UITextures[0].dataSize = stringBitmap->size;
    vc->UITextures[0].data = stringBitmap->data;

    VulkanPrepareTexture(
            &vc->gpu,
            &vc->device,
            &vc->setupCmd,
            &vc->cmdPool,
            &vc->queue,
            &vc->memoryProperties,
            useStagingBuffer,
            vc->UITextures,
            VK_IMAGE_LAYOUT_GENERAL);

    ARRAY_CREATE(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vdsi);
    vc->vdsi = vdsi;

    VulkanDescriptorSetInfo tempVulkanDescriptorSetInfo = {};
    tempVulkanDescriptorSetInfo.textureObjCount = DEMO_TEXTURE_COUNT;
    tempVulkanDescriptorSetInfo.uniformData = &vc->uniformData;
    tempVulkanDescriptorSetInfo.uniformDataFragment = &vc->uniformDataFragment;

    tempVulkanDescriptorSetInfo.descSet = {};
    tempVulkanDescriptorSetInfo.textureObj = (TextureObject *)AllocateMemory(reservedMemory, sizeof(TextureObject));
    memset(tempVulkanDescriptorSetInfo.textureObj, 0, sizeof(TextureObject));
    tempVulkanDescriptorSetInfo.name = "awesomeface";
    tempVulkanDescriptorSetInfo.imagePath ="./materials/textures/awesomeface.png";
    ARRAY_PUSH(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vc->vdsi, tempVulkanDescriptorSetInfo);

    tempVulkanDescriptorSetInfo.descSet = {};
    tempVulkanDescriptorSetInfo.textureObj = (TextureObject *)AllocateMemory(reservedMemory, sizeof(TextureObject));
    tempVulkanDescriptorSetInfo.name = "arche";
    tempVulkanDescriptorSetInfo.imagePath = "./materials/textures/arche.png";
    ARRAY_PUSH(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vc->vdsi, tempVulkanDescriptorSetInfo);

    tempVulkanDescriptorSetInfo.descSet = {};
    tempVulkanDescriptorSetInfo.textureObj = (TextureObject *)AllocateMemory(reservedMemory, sizeof(TextureObject));
    tempVulkanDescriptorSetInfo.name = "box";
    tempVulkanDescriptorSetInfo.imagePath = "./materials/textures/container.png";
    ARRAY_PUSH(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vc->vdsi, tempVulkanDescriptorSetInfo);

    tempVulkanDescriptorSetInfo.descSet = {};
    tempVulkanDescriptorSetInfo.textureObj = (TextureObject *)AllocateMemory(reservedMemory, sizeof(TextureObject));
    tempVulkanDescriptorSetInfo.name = "pshroom";
    tempVulkanDescriptorSetInfo.imagePath = "./materials/textures/pshroom.png";
    ARRAY_PUSH(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vc->vdsi, tempVulkanDescriptorSetInfo);

    tempVulkanDescriptorSetInfo.descSet = {};
    tempVulkanDescriptorSetInfo.textureObj = (TextureObject *)AllocateMemory(reservedMemory, sizeof(TextureObject));
    tempVulkanDescriptorSetInfo.name = "white";
    tempVulkanDescriptorSetInfo.imagePath = "";
    tempVulkanDescriptorSetInfo.textureObj[0].texWidth = gameMetadata->whiteBitmap.width;
    tempVulkanDescriptorSetInfo.textureObj[0].texHeight = gameMetadata->whiteBitmap.height;
    /* stbi does not do padding, so the pitch is the component times
     * the width of the image. The component is 4 because of STBI_rgb_alpha.
     */
    tempVulkanDescriptorSetInfo.textureObj[0].texPitch = gameMetadata->whiteBitmap.width * 4;
    tempVulkanDescriptorSetInfo.textureObj[0].dataSize =
        gameMetadata->whiteBitmap.width * gameMetadata->whiteBitmap.height * 4;
    tempVulkanDescriptorSetInfo.textureObj[0].data = gameMetadata->whiteBitmap.data;
    ARRAY_PUSH(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vc->vdsi, tempVulkanDescriptorSetInfo);

    tempVulkanDescriptorSetInfo.descSet = {};
    tempVulkanDescriptorSetInfo.textureObj = (TextureObject *)AllocateMemory(reservedMemory, sizeof(TextureObject));
    tempVulkanDescriptorSetInfo.name = "font";
    tempVulkanDescriptorSetInfo.imagePath = "";
    tempVulkanDescriptorSetInfo.textureObj[0].texWidth = gameMetadata->fontBitmap.width;
    tempVulkanDescriptorSetInfo.textureObj[0].texHeight = gameMetadata->fontBitmap.height;
    /* stbi does not do padding, so the pitch is the component times
     * the width of the image. The component is 4 because of STBI_rgb_alpha.
     */
    tempVulkanDescriptorSetInfo.textureObj[0].texPitch = gameMetadata->fontBitmap.width * 4;
    tempVulkanDescriptorSetInfo.textureObj[0].dataSize =
        gameMetadata->fontBitmap.width * gameMetadata->fontBitmap.height * 4;
    tempVulkanDescriptorSetInfo.textureObj[0].data = gameMetadata->fontBitmap.data;
    ARRAY_PUSH(VulkanDescriptorSetInfo, &gameMetadata->reservedMemory, vc->vdsi, tempVulkanDescriptorSetInfo);

    for(memory_index i = 0; i < ARRAY_LIST_SIZE(vc->vdsi); i++)
    {
        VulkanLoadImageToGPU(vc, &vc->vdsi[i]);
    }


}

void SetVulkanDescriptorSet(VulkanContext *vc, GameMetadata *gameMetadata, Bitmap *stringBitmap)
{
    for (memory_index i = 0; i < ARRAY_LIST_SIZE(vc->vdsi); i++)
    {
        VulkanSetDescriptorSet(vc, &vc->vdsi[i]);
    }

    VulkanSetDescriptorSet(
            vc,
            &vc->secondDescSet,
            vc->UITextures,
            DEMO_TEXTURE_COUNT,
            &vc->uniformData,
            &vc->uniformDataFragment);

    for(memory_index i = 0; i < ARRAY_LIST_SIZE(vc->vdsi); i++)
    {
        HashAdd(
                gameMetadata->hashBitmapVkDescriptorSet,
                FindBitmap(&gameMetadata->bitmapSentinelNode, vc->vdsi[i].name),
                &vc->vdsi[i].descSet);
    }

    HashAdd(
            gameMetadata->hashBitmapVkDescriptorSet,
            stringBitmap,
            &vc->secondDescSet);
}

void SetPlayer(GameMetadata *gm)
{
    GameMemory *reservedMemory = &gm->reservedMemory;
    Entity newPlayer = {};
    newPlayer.position = glm::vec3(0, 0, 0);
    newPlayer.isPlayer = true;

    u32 index = Append(reservedMemory, g_entityManager, &newPlayer);

    /* NOTE: we don't need to free the player since we created it in the
     * stack
     */
    v3 pos = { 0, 0, 0.01f };
    g_player = &g_entityManager->entities[index];
    g_player->type = 2;
    gm->playerRect = CreateRectangle(reservedMemory, pos, COLOR_WHITE, 2, 1);
    AssociateEntity(gm->playerRect, g_player, false);
    g_rectManager->player = gm->playerRect;
    gm->playerRect->type = REGULAR;
    gm->playerRect->renderLayer = PLAYER;
    gm->playerRect->frameDirection = LEFT;
    gm->playerRect->bitmap = FindBitmap(&gm->bitmapSentinelNode, "arche");
    gm->playerRect->bitmapID = gm->playerRect->bitmap->bitmapID;
}

void SetParticle(GameMetadata *gm)
{
    GameMemory *reservedMemory = &gm->reservedMemory;

    f32 base = -1.01f;
    f32 width = 0.05f;
    f32 height = 0.05f;
#if 1
    Bitmap *bitmap = FindBitmap(&gm->bitmapSentinelNode, (memory_index)0);
#else
    Bitmap *bitmap = FindBitmap(&gm->bitmapSentinelNode, "box");
#endif
    gm->particleSystem.particleCount = 10;
    gm->particleSystem.particles =
        (Particle *)AllocateMemory(reservedMemory, gm->particleSystem.particleCount * sizeof(Particle));

    for (memory_index i = 0; i < gm->particleSystem.particleCount; i++)
    {
        Particle *particle = &gm->particleSystem.particles[i];
        ZeroSize(particle, sizeof(Particle));
        particle->TTL = particle->maxTTL = 60000;

        v3 basePosition = { base, base, 0.5f };
        base += 0.15f;
        v4 color = { 0.1f, 0.1f, 0.1f, 1 };
        particle->rect.color = color;
        particle->rect.bitmapID = 0;
        particle->rect.bitmap = bitmap;
        particle->rect.renderLayer = BEHIND_PLAYER;
        particle->positionOffset = basePosition;
        SetRectPoints(&particle->rect, basePosition, width, height);
    }

}

void DoVulkanDepthStencil(VulkanContext *vc)
{
    if (vc->depthStencil > 0.99f)
    {
        vc->depthIncrement = -0.001f;
    }
    if (vc->depthStencil < 0.8f)
    {
        vc->depthIncrement = 0.001f;
    }

    vc->depthStencil += vc->depthIncrement;
}

void DoVukanUpdateToNextFrame(VulkanContext *vc)
{
    //Wait for work to finish before updating MVP.
    vkDeviceWaitIdle(vc->device);
    vc->curFrame++;
    if (vc->frameCount != INT32_MAX && vc->curFrame == vc->frameCount)
    {
        vc->quit = true;
    }
}

void SetPerFrameData(GameMetadata *gameMetadata, GameMemory *perFrameMemory)
{
    gameMetadata->perFrameRenderGroup = CreateRenderGroup(perFrameMemory, 6 /*numOfPointsPerRect*/, 20001 /*maxEntityCount*/);
    gameMetadata->hitBoxes = CreateRectDynamicArray(perFrameMemory, 100);
    gameMetadata->hurtBoxes = CreateRectDynamicArray(perFrameMemory, 100);
    gameMetadata->rdaDebug = CreateRectDynamicArray(perFrameMemory, 10000);
    gameMetadata->rdaDebugUI = CreateRectDynamicArray(perFrameMemory);
    gameMetadata->sm = CreateSceneManager(gameMetadata, perFrameMemory);
    SetAABB(&g_rectManager->NonTraversable);
    CreateScenePartition(gameMetadata->sm, &g_rectManager->NonTraversable);
}

void InitGameUpdateAndRender(VulkanContext *vc, GameMetadata *gameMetadata)
{
    START_DEBUG_TIMING();
    stbi_set_flip_vertically_on_load(1);

    GameMemory *reservedMemory = &gameMetadata->reservedMemory;
    GameTimestep **gameTimestep = &gameMetadata->gameTimestep;

    ASSERT(!g_reservedMemory);
    ASSERT(!g_rectManager);
    ASSERT(!*gameTimestep);
    ASSERT(!g_camera);
    ASSERT(!g_eda);
    ASSERT(!g_projection);
    ASSERT(!g_entityManager);

    g_reservedMemory = reservedMemory;

    /* TODO: May be the entity manager should be the only one creating the
     * entities?
     */
    g_entityManager = CreateEntityManger(reservedMemory);
    g_eda = CreateEntityDynamicArray(reservedMemory);
    g_rectManager = CreateRectManager(reservedMemory);
    gameMetadata->rectManager = g_rectManager;

    LoadAssets(gameMetadata);

    ARRAY_CREATE(glm::vec3, &gameMetadata->reservedMemory, worldArr);
    gameMetadata->objectsToBeAddedTotheWorld = worldArr;

    SetFont(gameMetadata);
    SetHash(gameMetadata);

    g_vkBuffers.count = 0;
    g_vkBuffers.maxNum = 100;

    /* Creating pipeline layout, descriptor pool, and render pass can be done
     * indenpendently
     */
    VulkanPrepareDescriptorPool(vc);

    vc->pipelineLayout = {};
    VulkanInitPipelineLayout(vc);

    Bitmap stringBitmap = {};
    SetBitmapToGPUForPipeline(vc, gameMetadata, &stringBitmap);

    /* XXX: This is needed so that we can bind a descriptor set to a pipeline.
     * There might be a better way of doing this. This is just a hack.*/
    vc->descSet = &vc->vdsi[0].descSet;
    SetVulkanDescriptorSet(vc, gameMetadata, &stringBitmap);

    VulkanInitRenderPass(vc);
    VulkanInitFrameBuffers(vc);
    SetPreparePipeline(vc);

    SetGameTimeStep(gameMetadata);
    SetCamera(gameMetadata);
    SetPerspectiveProjection(gameMetadata);
    SetPlayer(gameMetadata);
    SetParticle(gameMetadata);

    LoadStuff(gameMetadata);

    gameMetadata->initFromGameUpdateAndRender = true;
    END_DEBUG_TIMING();

}
