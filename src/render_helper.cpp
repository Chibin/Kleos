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
        gm->camera = CreateCamera(&gm->reservedMemory, cameraPos, cameraTarget, cameraUp);
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
    gm->projection = (glm::mat4 *)AllocateMemory(&gm->reservedMemory, (sizeof(glm::mat4)));
    *gm->projection =
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
    HASH_CREATE(HashEntityRect, &gm->reservedMemory, hashEntityRect, MAX_HASH);
    HASH_CREATE(HashEntityMovement, &gm->reservedMemory, hashEntityMovement, MAX_HASH);

    gm->hashBitmapVkDescriptorSet = hashBMPDescSet;
    gm->hashBitmap = hashBitmap;
    gm->hashEntityRect = hashEntityRect;
    gm->hashEntityMovement = hashEntityMovement;
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
    v3 pos = { 0, 0, 0.01f };
    Entity *playerEntity = (Entity *)AllocateMemory0(reservedMemory, sizeof(Entity));
    playerEntity->id = g_entityID++;
    playerEntity->isPlayer = true;
    gm->playerEntity = playerEntity;

    Rect *playerRect = CreateRectangle(reservedMemory, pos, COLOR_WHITE, 2, 1);
    playerRect->type = REGULAR;
    playerRect->renderLayer = PLAYER;
    playerRect->frameDirection = LEFT;
    playerRect->bitmap = FindBitmap(&gm->bitmapSentinelNode, "arche");
    playerRect->bitmapID = playerRect->bitmap->bitmapID;
    playerRect->spriteAnimation =
        GetSpriteAnimationInfo(
                GetFrameAnimation(&gm->frameAnimationSentinelNode, "arche.png"),
                "IDLE");

    Movement *movement = (Movement *)AllocateMemory0(reservedMemory, sizeof(Movement));
    movement->position.x = playerRect->center.x;
    movement->position.y = playerRect->center.y;
    movement->position.z = playerRect->center.z;

    HashAdd(gm->hashEntityRect, playerEntity, playerRect);
    HashAdd(gm->hashEntityMovement, playerEntity, movement);
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

        v3 center = { base, base, 0.5f };
        base += 0.15f;
        v4 color = { 0.1f, 0.1f, 0.1f, 1 };
        particle->rect.color = color;
        particle->rect.bitmapID = 0;
        particle->rect.bitmap = bitmap;
        particle->rect.renderLayer = BEHIND_PLAYER;
        particle->positionOffset = center;
        SetRectPoints(&particle->rect, center, width, height);
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
        ProcessKeysHeldDown(
                gameMetadata->hashEntityMovement,
                gameMetadata->playerEntity,
                keystate);
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
            ProcessMouseEditMode(gameMetadata, gameMetadata->camera, gameMetadata->projection, event);
        case SDL_MOUSEWHEEL:
            ProcessMouseInput(event, gameMetadata->camera);
            break;
        case SDL_MOUSEMOTION:
            g_mousePoint = ProcessMouseMotion(event.motion);
            UpdateMouseDrag(gameMetadata, gameMetadata->camera, gameMetadata->projection, event);
            break;
        case SDL_KEYDOWN:
            ProcessInputDown(
                    event.key.keysym.sym,
                    gameMetadata,
                    gameMetadata->camera,
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

void SetPerFrameData(GameMetadata *gameMetadata, GameMemory *perFrameMemory, Camera *camera, glm::mat4 *projection)
{

    /* bottom left (0,0 starts at the top left) */
    v2 screenCoordinatesMin = v2{0, SCREEN_HEIGHT};
    /* top right */
    v2 screenCoordinatesMax = v2{SCREEN_WIDTH, 0};
    glm::vec3 worldPosMin = GetWorldPointFromMouse(
            camera,
            projection,
            screenCoordinatesMin,
            gameMetadata->screenResolution,
            gameMetadata->infinitePlaneNormal);
    glm::vec3 worldPosMax = GetWorldPointFromMouse(
            camera,
            projection,
            screenCoordinatesMax,
            gameMetadata->screenResolution,
            gameMetadata->infinitePlaneNormal);
    MinMax minMax = {V2(worldPosMin), V2(worldPosMax)};

    const u32 numOfPointsPerRect = 6;
    const u16 maxEntityCount = 20001;
    gameMetadata->perFrameRenderGroup = CreateRenderGroup(perFrameMemory, numOfPointsPerRect, maxEntityCount, minMax);
    gameMetadata->hitBoxes = CreateRectDynamicArray(perFrameMemory, 100);
    gameMetadata->hurtBoxes = CreateRectDynamicArray(perFrameMemory, 100);
    gameMetadata->rdaDebug = CreateRectDynamicArray(perFrameMemory, 10000);
    gameMetadata->rdaDebugUI = CreateRectDynamicArray(perFrameMemory);
    gameMetadata->sm = CreateSceneManager(gameMetadata, perFrameMemory);
    SetAABB(&g_rectManager->NonTraversable);
    CreateScenePartition(gameMetadata->sm, &g_rectManager->NonTraversable);
}

void LoadStuff(GameMetadata *gameMetadata)
{
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;

    v4 color = { 0.1f, 0.1f, 0.1f, 1.0f };
    /* load random data */
    for (int i = 0; i < 100; i++)
    {
        for (int y = 0; y < 100; y++)
        {
            v3 startingPosition = { -1 + (real32)i, 1 * (real32)y, 0 };
            Rect *r =
                CreateRectangle(reservedMemory, startingPosition, color, 1, 1);
            r->bitmapID = FindBitmap(&gameMetadata->bitmapSentinelNode, "awesomeface")->bitmapID;
            r->renderLayer = BACKGROUND;
            PushBack(&(g_rectManager->Traversable.rda), r);
        }
    }

    /* Spawn new enemty */
    g_enemyNPC = (NPC *)AllocateMemory(reservedMemory, sizeof(NPC));
    memset(g_enemyNPC, 0, sizeof(NPC));

    g_enemyNPC->dim.height = 1.0f;
    g_enemyNPC->dim.width = 1.5f;
    g_enemyNPC->bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, "pshroom");
    g_enemyNPC->spriteAnimation =
        CreateCopyOfSpriteAnimationInfo(
                reservedMemory,
                GetSpriteAnimationInfo(
                    GetFrameAnimation(&gameMetadata->frameAnimationSentinelNode, "pshroom"),
                    "IDLE")
        );
    g_enemyNPC->spriteAnimation->direction = LEFT;
    g_enemyNPC->direction = LEFT;
    g_enemyNPC->renderLayer = BEHIND_PLAYER;
    g_enemyNPC->movementType = X_MOVEMENT;
    g_enemyNPC->movementPattern = UNI_DIRECTIONAL;
}

inline void LoadAssets(GameMetadata *gameMetadata)
{

    GameMemory *reservedMemory = &gameMetadata->reservedMemory;

    auto *awesomefaceBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));
    auto *newBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));
    auto *boxBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));
    auto *pshroomBitmap = (Bitmap *)AllocateMemory(reservedMemory, sizeof(Bitmap));

    SetBitmap(awesomefaceBitmap, "awesomeface", TextureParam{ GL_LINEAR, GL_LINEAR },
              g_bitmapID++, "./materials/textures/awesomeface.png");
    SetBitmap(newBitmap, "arche", TextureParam{ GL_NEAREST, GL_NEAREST },
              g_bitmapID++, "./materials/textures/arche.png");
    SetBitmap(boxBitmap, "box", TextureParam{ GL_LINEAR, GL_LINEAR },
              g_bitmapID++, "./materials/textures/container.png");
    SetBitmap(pshroomBitmap, "pshroom", TextureParam{ GL_LINEAR, GL_LINEAR },
              g_bitmapID++, "./materials/textures/pshroom.png");

    PushBitmap(&gameMetadata->bitmapSentinelNode, awesomefaceBitmap);
    PushBitmap(&gameMetadata->bitmapSentinelNode, newBitmap);
    PushBitmap(&gameMetadata->bitmapSentinelNode, boxBitmap);
    PushBitmap(&gameMetadata->bitmapSentinelNode, pshroomBitmap);

    PushBitmap(&gameMetadata->bitmapSentinelNode, &gameMetadata->whiteBitmap);
    PushBitmap(&gameMetadata->bitmapSentinelNode, &gameMetadata->fontBitmap);

    Bitmap *archeBitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, newBitmap->bitmapID);
    ASSERT(archeBitmap != nullptr);

    /* XXX: There's a loose coupling between frame animation and bitmap width
     * and height. We need the width and height to make a conversion to pixel
     * to UV coordinates
     */
    u32 bitmapWidth = archeBitmap->width;
    u32 bitmapHeight = archeBitmap->height;

    LoadFrameData(gameMetadata, "./assets/texture_data/frames.txt");

    FrameAnimation *fa = GetFrameAnimation(&gameMetadata->frameAnimationSentinelNode, "arche.png");
    ASSERT(fa != NULL);

    for (memory_index animCount = 0; animCount < fa->animationCount; animCount ++)
    {

        /* TODO: Have a way to find the correct animation information based on name? */
        ASSERT(fa->frameCycles[animCount].animationInfo == NULL);
        if (fa->frameCycles[animCount].animationInfo == nullptr)
        {
            fa->frameCycles[animCount].animationInfo =
                (Animation2D *)AllocateMemory(reservedMemory, sizeof(Animation2D));
            ZeroSize(fa->frameCycles[animCount].animationInfo, sizeof(Animation2D));
        }

        Animation2D *spriteAnim = fa->frameCycles[animCount].animationInfo;
        spriteAnim->direction = LEFT;
        u32 totalFrames = fa->frameCycles[animCount].frameCount;
        spriteAnim->totalFrames = totalFrames;
        spriteAnim->frameCoords =
            (RectUVCoords *)AllocateMemory(reservedMemory, sizeof(RectUVCoords) * totalFrames);
        if (strcmp(fa->frameCycles[animCount].name, "ATTACK") == 0)
        {
            spriteAnim->timePerFrame = 150;
        }
        else if (strcmp(fa->frameCycles[animCount].name, "IDLE") == 0)
        {
            spriteAnim->timePerFrame = 1000 * 0.75;
        }

        /* TODO: Need to figure out how to use compound literals with specific assignment in windows */
        v2 topRight = {}, bottomRight = {}, bottomLeft = {}, topLeft = {};
        for(memory_index i = 0; i < totalFrames; i++)
        {
            spriteAnim->frameCoords[i] =
                RectUVCoords{
                    topRight    = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[0], bitmapWidth, bitmapHeight),
                    bottomRight = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[1], bitmapWidth, bitmapHeight),
                    bottomLeft  = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[2], bitmapWidth, bitmapHeight),
                    topLeft     = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[3], bitmapWidth, bitmapHeight)
                };
        }
    }

    fa = GetFrameAnimation(&gameMetadata->frameAnimationSentinelNode, "pshroom");
    ASSERT(fa != NULL);
    Bitmap *bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, "pshroom");
    bitmapWidth = bitmap->width;
    bitmapHeight = bitmap->height;

    for (memory_index animCount = 0; animCount < fa->animationCount; animCount ++)
    {

        /* TODO: Have a way to find the correct animation information based on name? */
        ASSERT(fa->frameCycles[animCount].animationInfo == NULL);
        if (fa->frameCycles[animCount].animationInfo == nullptr)
        {
            fa->frameCycles[animCount].animationInfo =
                (Animation2D *)AllocateMemory(reservedMemory, sizeof(Animation2D));
            ZeroSize(fa->frameCycles[animCount].animationInfo, sizeof(Animation2D));
        }

        Animation2D *spriteAnim = fa->frameCycles[animCount].animationInfo;
        spriteAnim->direction = LEFT;
        u32 totalFrames = fa->frameCycles[animCount].frameCount;
        spriteAnim->totalFrames = totalFrames;
        spriteAnim->frameCoords =
            (RectUVCoords *)AllocateMemory(reservedMemory, sizeof(RectUVCoords) * totalFrames);
        if (strcmp(fa->frameCycles[animCount].name, "ATTACK") == 0)
        {
            spriteAnim->timePerFrame = 150;
        }
        else if (strcmp(fa->frameCycles[animCount].name, "IDLE") == 0)
        {
            spriteAnim->timePerFrame = 1000 * 0.75;
        }

        /* TODO: Need to figure out how to use compound literals with specific assignment in windows */
        v2 topRight = {}, bottomRight = {}, bottomLeft = {}, topLeft = {};
        for(memory_index i = 0; i < totalFrames; i++)
        {
            spriteAnim->frameCoords[i] =
                RectUVCoords{
                    topRight    = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[0], bitmapWidth, bitmapHeight),
                    bottomRight = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[1], bitmapWidth, bitmapHeight),
                    bottomLeft  = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[2], bitmapWidth, bitmapHeight),
                    topLeft     = PixelToUV(fa->frameCycles[animCount].frames[i].pixel[3], bitmapWidth, bitmapHeight)
                };
        }
    }

    MapData *rootMDNode = LoadAssetMap("./assets/map_data/first_map.txt");
    ASSERT(rootMDNode->next != nullptr);

    for(MapData *currentNode = rootMDNode->next; currentNode != nullptr; currentNode = currentNode->next)
    {
        v4 color = { 0.1f, 0.1f, 0.1f, 1.0f };
        for (memory_index i = 0; i < currentNode->count; i++)
        {

            v3 startingPosition = V3(currentNode->basePoints[i], 0);
            Rect *collissionRect =
                CreateRectangle(reservedMemory, startingPosition, color, currentNode->dim);

            collissionRect->type = COLLISION;
            collissionRect->renderLayer = FRONT_STATIC;
            UpdateColors(collissionRect, v4{ 0.0f, 0.0f, 1.0f, 0.7f });

            Bitmap *findBoxBitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, "box");
            collissionRect->bitmapID = findBoxBitmap->bitmapID;
            PushBack(&(g_rectManager->NonTraversable.rda), collissionRect);
        }
    }
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
    ASSERT(!g_eda);
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

    gameMetadata->infinitePlaneNormal = glm::vec3(0, 0, 1);

    gameMetadata->initFromGameUpdateAndRender = true;
    END_DEBUG_TIMING();

}
