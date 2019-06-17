#ifndef __UPDATE__
#define __UPDATE__

void UpdateMovement(GameMetadata *gameMetadata, Movement *movement, v2 dim, f32 dt)
{
    const GLfloat gravity = -9.81f;
    //Entity *e = gameMetadata->playerEntity;
    //Movement *movement = HashGetValue(HashEntityMovement, gameMetadata->hashEntityMovement, e);
    //Rect *rectFromEntity = HashGetValue(HashEntityRect, gameMetadata->hashEntityRect, e);

    //      if (IntersectionAABB(rect, v2{e->position.x, e->position.y},
    //      rayDirection))
    v2 center = 0.5f * dim;
    Rect nextUpdate = {};
    nextUpdate.min = V2(movement->position) + V2(movement->velocity) * dt + 0.5f * V2(movement->acceleration) * dt * dt - center;
    nextUpdate.max = V2(movement->position) + V2(movement->velocity) * dt + 0.5f * V2(movement->acceleration) * dt * dt + center;

    /* XXX: If we're on the ground we should always reset the accelration to the force of gravity. */
    movement->acceleration.y += gravity;

    MinMax temp = GetMinMax(&nextUpdate);
    AABB range = MinMaxToSquareAABB(&temp);
#if 1
    f32 arbitraryPadding = 5.0f;
#else
    f32 arbitraryPadding = 50.0f;
#endif
    range.halfDim = range.halfDim + arbitraryPadding;
    Rect **arr = GetRectsWithInRange(gameMetadata->sm, &range);

#if 0
    AddDebugRect(gameMetadata, &range, COLOR_GREEN_TRANSPARENT);
#endif

    AABB uiTest = {};
    glm::vec3 rayWorld =
        UnProject(gameMetadata->camera, gameMetadata->projection, V2(g_mousePoint), gameMetadata->screenResolution);

    f32 dimRange = 0.1f;
    uiTest.halfDim = v2{dimRange, dimRange * (gameMetadata->screenResolution.x / gameMetadata->screenResolution.y)};
    uiTest.center = V2(ScreenSpaceToNormalizedDeviceSpace(V2(g_mousePoint), gameMetadata->screenResolution));
    AddDebugRectUI(gameMetadata, &uiTest, COLOR_RED_TRANSPARENT);

    /* This points towards us */
    glm::vec3 infinitePlaneNormal = glm::vec3(0, 0, 1);

    for(memory_index i = 0; i < ARRAY_LIST_SIZE(arr); i++)
    {
        Rect *rect = arr[i];
#if 0
        AddDebugRect(gameMetadata, rect, COLOR_YELLOW_TRANSPARENT);
#endif

        glm::vec3 rayDirection = glm::vec3(1 / rayWorld.x, 1 / rayWorld.y, 0);
        if (IntersectionAABB(rect, V2(gameMetadata->camera->pos), rayDirection))
        {
            AddDebugRect(gameMetadata, rect, COLOR_BLACK_TRANSPARENT);
        }
#if 0
        real32 dX = e->velocity.x * dt;
        real32 dY = e->velocity.y + (gravity + e->acceleration.y) * dt;

        glm::vec3 rayDirection = { dX, dY, 0 };
        rayDirection = glm::normalize(rayDirection);
        rayDirection = glm::vec3(1 / rayDirection.x, 1 / rayDirection.y, 0);
#endif
        if (rect->type == COLLISION && TestAABBAABB(rect, &nextUpdate))
        {
            /* how to differentiate between x and y axis? */
            movement->position.y = rect->max.y + dim.y * 0.5f;
            movement->velocity.y = 0;
            movement->acceleration.y = gravity;
        }
    }

    /* TODO: There is a bug here. We're not properly updating the position
     * based on the collisions
     */
    movement->position += movement->velocity * dt + 0.5f * movement->acceleration * dt * dt;
    movement->velocity.y += movement->acceleration.y *dt;
}

void UpdateAttack(
        GameMetadata *gameMetadata, GameTimestep *gt,
        Entity *e, Movement *movement, Rect *rectFromEntity)
{
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;

    /* This is related to hit boxes of the frames */
    if (e->frameState.startFrame == NULL)
    {
        e->frameState.startFrame =
            (FrameData *)AllocateMemory(reservedMemory, sizeof(FrameData));

        e->frameState.startFrame->duration = 170;
        e->frameState.startFrame->dim = v2{0.35f, 0.175f};
        e->frameState.startFrame->pos = v3{1.5, 0, 0};
        e->frameState.startFrame->next = NULL;

        e->frameState.currentFrame = e->frameState.startFrame;

        /* TODO: This should be somewhere else */
        e->frameState.transform = glm::mat4(-2.0f);
        e->frameState.transform[3][3] = 1.0f;

        if (rectFromEntity->frameDirection == RIGHT)
        {
            e->frameState.transform[0][0] = abs(e->frameState.transform[0][0]);
            e->frameState.transform[1][1] = abs(e->frameState.transform[1][1]);
        }
    }

    if (e->frameState.timePassedCurrentFrame == 0)
    {
        rectFromEntity->spriteAnimation =
            GetSpriteAnimationInfo(
                    GetFrameAnimation(&gameMetadata->frameAnimationSentinelNode, "arche.png"),
                    "ATTACK");
    }

    //AttackInfo attackInfo = GetAttackFrameInfo();
    if (UpdateFrameState(&e->frameState, gt->deltaTime))
    {
        /* TODO: The position that's passed needs to be the center of the
         * movement. Otherwise, it's hard to make the frame distance
         * symetrical when swapping between both directions
         */
        AddDebugRect(gameMetadata, &e->frameState, V3(movement->position), COLOR_RED_TRANSPARENT);
        AABB range = CreateFrameAABB(&e->frameState, V3(movement->position));
        FOR_EACH_HASH_KEY_VAL_BEGIN(HashEntityNPC, hashKeyVal, gameMetadata->hashEntityNPC)
        {
            NPC *npc = hashKeyVal->val;
            AABB npcAABB = {};
            npcAABB.center = V2(npc->movement->position);
            npcAABB.halfDim = npc->dim * 0.5f;
            if (TestAABBAABB(&range, &npcAABB))
            {
                if (movement->position.x < npc->movement->position.x)
                {
                    npc->movement->velocity.x += 2.0f;
                }
                else
                {
                    npc->movement->velocity.x -= 2.0f;
                }

                npc->movement->velocity.y += 1.0f;
            }
        }
        FOR_EACH_HASH_KEY_VAL_END();
    }
    else
    {
        e->willAttack = false;
    }

}

void UpdatePlayer(
        GameMetadata *gameMetadata, GameTimestep *gt, Entity *e, Movement *movement, Rect *rectFromEntity,
        f32 dt, RenderGroup *perFrameRenderGroup)
{
    const GLfloat gravity = -9.81f;

    if (rectFromEntity->spriteAnimation->currentFrameIndex == 0 && e->willAttack == false)
    {
        rectFromEntity->spriteAnimation =
            GetSpriteAnimationInfo(
                    GetFrameAnimation(&gameMetadata->frameAnimationSentinelNode, "arche.png"),
                    "IDLE");
    }

    /* TODO: bound checking for the camera such that we only move the camera
     * when necessary
     */
    //UpdateEntityFrameDirection();
    if (movement->velocity.x > 0)
    {
        rectFromEntity->frameDirection = RIGHT;
        e->frameState.transform[0][0] = abs(e->frameState.transform[0][0]);
        e->frameState.transform[1][1] = abs(e->frameState.transform[1][1]);
    }
    else if (movement->velocity.x < 0)
    {
        rectFromEntity->frameDirection = LEFT;
        e->frameState.transform[0][0] = -1 * abs(e->frameState.transform[0][0]);
        e->frameState.transform[1][1] = -1 * abs(e->frameState.transform[1][1]);
    }
    /* else don't update */

    /* follow the character around */
    CameraUpdateTarget(gameMetadata->camera, movement->position);
    UpdatePosition(rectFromEntity, V3(movement->position));

    UpdateCurrentFrame(rectFromEntity->spriteAnimation, 17.6f);
    UpdateUV(rectFromEntity, *rectFromEntity->spriteAnimation->currentFrame);

    for (memory_index i = 0; i < gameMetadata->particleSystem.particleCount; i++)
    {
        Particle *particle = &gameMetadata->particleSystem.particles[i]; // NOLINT

        v3 newPosition = V3(movement->position);

        particle->acc.y = gravity * 2;
        particle->vel.x = 0;
        UpdateParticlePosition(particle, dt);
        if (particle->positionOffset.y <= rectFromEntity->min.y)
        {
            particle->positionOffset.y = newPosition.y;
            particle->acc.y = 0;
            UpdatePosition(&particle->rect, particle->positionOffset);
        }
        else
        {
            UpdatePosition(&particle->rect, particle->positionOffset + newPosition);
        }

    }
    UpdateAndGenerateParticleRectInfo(perFrameRenderGroup,
            &gameMetadata->particleSystem,
            (s32)gameMetadata->gameTimestep->deltaTime);
}

void UpdateEntities(GameMetadata *gameMetadata, GameTimestep *gt, RectDynamicArray *hitBoxes, RectDynamicArray *hurtBoxes, RenderGroup *perFrameRenderGroup)
{
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;

    /*
     * Attack animation startup
     * Create animation hitboxes
     * Create animation hurtboxes
     * Update movement location
     */

    /* NOTE: Look at this later Axis-Aligned Bounding Box*/
    // position, velocity, acceleration
    const GLfloat gravity = -9.81f;

    f32 dt = gt->dt;

    FOR_EACH_HASH_KEY_VAL_BEGIN(HashSetEntity, hashKeyVal, gameMetadata->hashSetEntity)
    {
        Entity *e = hashKeyVal->key;
        Movement *movement = HashGetValue(HashEntityMovement, gameMetadata->hashEntityMovement, e);
        Rect *rectFromEntity = HashGetValue(HashEntityRect, gameMetadata->hashEntityRect, e);

        if (rectFromEntity != NULL)
        {
            UpdateMovement(gameMetadata, movement, rectFromEntity->dim, dt);
        }
        else
        {

            NPC *npcFromEntity = HashGetValue(HashEntityNPC, gameMetadata->hashEntityNPC, e);
            ASSERT(npcFromEntity != NULL);
            UpdateMovement(gameMetadata, movement, npcFromEntity->dim, dt);
        }

        /* TODO: can't let the player spam attack while we're still in an attack animation */
        if (e->willAttack && rectFromEntity != NULL)
        {
            UpdateAttack(gameMetadata, gt, e, movement, rectFromEntity);
        }

        for (int i = 0; i < hitBoxes->size; i++)
        {
            Rect *rect = hitBoxes->rects[i];
            for (int y = 0; y < hurtBoxes->size; y++)
            {
                Rect hb = *(hurtBoxes->rects[y]);
                if (TestAABBAABB(rect, &hb))
                {
                    printf("CHECKING hitboxes\n");
                }
            }
        }

        if (e->isPlayer)
        {
            UpdatePlayer(gameMetadata, gt, e, movement, rectFromEntity, dt, perFrameRenderGroup);
        }

        /* Apply "friction" */
        movement->velocity.x = 0;

    }
    FOR_EACH_HASH_KEY_VAL_END();
}

b32 IsMouseInSelectedUIRegion(GameMetadata *gm, AABB range)
{
    if (gm->mouseInfo.mouseType == LEFT_SINGLE_CLICK || gm->mouseInfo.mouseType == LEFT_DRAG_CLICK)
    {
        v2 leftMouseButton = V2(ScreenSpaceToNormalizedDeviceSpace(
                    gm->mouseInfo.leftScreenCoordinates[0],
                    gm->screenResolution));

        return ContainsPoint(&range, leftMouseButton);
    }
    else if (gm->mouseInfo.mouseType == RIGHT_SINGLE_CLICK || gm->mouseInfo.mouseType == RIGHT_DRAG_CLICK)
    {
        v2 rightMouseButton = V2(ScreenSpaceToNormalizedDeviceSpace(
                    gm->mouseInfo.rightScreenCoordinates[0],
                    gm->screenResolution));
        return ContainsPoint(&range, rightMouseButton);
    }
    else if (gm->mouseInfo.mouseType == MOUSE_WHEEL_UP || gm->mouseInfo.mouseType == MOUSE_WHEEL_DOWN)
    {
        v2 middleMouseButton = V2(ScreenSpaceToNormalizedDeviceSpace(
                    gm->mouseInfo.middleScreenCoordinate,
                    gm->screenResolution));
        return ContainsPoint(&range, middleMouseButton);
    }

    return false;
}

void DoEditModeUI(GameMetadata *gameMetadata, RenderGroup *perFrameRenderGroup)
{
    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;

    UIInfo *uiInfo = HashGetValue(HashCharUIInfo, gameMetadata->hashCharUIInfo, "edit_ui_bar");
    PushToRenderGroup(perFrameRenderGroup, gameMetadata, perFrameMemory, uiInfo);

    if (gameMetadata->editMode.isCommandPrompt)
    {
        UIInfo *uiInfo = HashGetValue(HashCharUIInfo, gameMetadata->hashCharUIInfo, "command_prompt_ui");
        PushToRenderGroup(perFrameRenderGroup, gameMetadata, perFrameMemory, uiInfo);
    }
}

AABB DetermineAABBRangeBasedOnMouse(GameMetadata *gameMetadata)
{
    AABB result = {};
    glm::vec3 mouse0 = {};
    glm::vec3 mouse1 = {};
    if (gameMetadata->mouseInfo.mouseType == LEFT_SINGLE_CLICK ||
            gameMetadata->mouseInfo.mouseType == LEFT_DRAG_CLICK ||
            gameMetadata->mouseInfo.mouseType == LEFT_MOUSE_DRAG)
    {
        mouse0 =
            GetWorldPointFromMouse(gameMetadata, gameMetadata->mouseInfo.leftScreenCoordinates[0]);
        mouse1 =
            GetWorldPointFromMouse(gameMetadata, gameMetadata->mouseInfo.leftScreenCoordinates[1]);
    }
    else if (gameMetadata->mouseInfo.mouseType == RIGHT_SINGLE_CLICK)
    {
        mouse0 =
            GetWorldPointFromMouse(gameMetadata, gameMetadata->mouseInfo.rightScreenCoordinates[0]);
        mouse1 =
            GetWorldPointFromMouse(gameMetadata, gameMetadata->mouseInfo.rightScreenCoordinates[1]);
    }

    if (gameMetadata->mouseInfo.mouseType == LEFT_SINGLE_CLICK)
    {
        result.halfDim = v2{1.0f, 1.0f};
        result.center = V2(mouse0);
    }
    else if (gameMetadata->mouseInfo.mouseType == RIGHT_SINGLE_CLICK)
    {
        f32 arbitraryPadding = 50.0f;
        result.halfDim = v2{arbitraryPadding, arbitraryPadding};
        result.center = V2(mouse0);
    }
    else
    {
        result.halfDim = V2(mouse0 - mouse1) * 0.5f;
        result.center = V2(mouse0) - result.halfDim;
        /* sometimes we're going to the negative value when calculating the
         * dimension. This needs to be positive in order to work.
         * We also need to use the negative dimension to calculate two out of the four quadrants.
         */
        result.halfDim = abs(result.halfDim);
    }

    return result;
}

void UpdateBasedOnEditModeChanges(GameMetadata *gameMetadata)
{
    if (!gameMetadata->editMode.isActive)
    {
        return;
    }

    b32 buttonUIFound = false;
    b32 selectRectUI = false;

    if (gameMetadata->mouseInfo.isNew)
    {
        gameMetadata->mouseInfo.isNew = false;
        FOR_EACH_HASH_KEY_VAL_BEGIN(HashCharUIInfo, hashKeyVal, gameMetadata->hashCharUIInfo)
        {
            UIInfo *uiInfo = hashKeyVal->val;
            buttonUIFound = gameMetadata->editMode.selectedRect &&
                IsMouseInSelectedUIRegion(gameMetadata, uiInfo->range);
            if (buttonUIFound)
            {
                selectRectUI = strcmp(hashKeyVal->key, "select_rect_ui")  == 0;
                FOR_EACH_HASH_BREAK();
            }
        }
        FOR_EACH_HASH_KEY_VAL_END();
    }

    DoEditModeUI(gameMetadata, gameMetadata->perFrameRenderGroupUI);

    if (gameMetadata->editMode.selectedRect != nullptr)
    {
        if (buttonUIFound && selectRectUI)
        {
            if (gameMetadata->mouseInfo.mouseType == MOUSE_WHEEL_UP)
            {
                gameMetadata->editMode.selectUITextureOffset.y -= 0.1f;

            }
            else if (gameMetadata->mouseInfo.mouseType == MOUSE_WHEEL_DOWN)
            {
                gameMetadata->editMode.selectUITextureOffset.y += 0.1f;
            }
        }

        GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;
        UIInfo *uiInfo = HashGetValue(HashCharUIInfo, gameMetadata->hashCharUIInfo, "select_rect_ui");
        PushToRenderGroup(gameMetadata->perFrameRenderGroupUI, gameMetadata, perFrameMemory, uiInfo);

        b32 skipFilter = true;

        Bitmap *sentinelNode = &gameMetadata->bitmapSentinelNode;
        v3 position = V3(uiInfo->range.center, 0);
        /* This will make the first texture at the very top of the screen. */
        position.y = 0.75f;
        position += gameMetadata->editMode.selectUITextureOffset;

        for(Bitmap *node = sentinelNode->next; node != sentinelNode; node = node->next)
        {
            if (strcmp(node->name, "font") == 0 || strcmp(node->name, "arche") == 0 ||
                    strcmp(node->name, "pshroom") == 0 || strcmp(node->name, "white") == 0)
            {
                continue;
            }
            Rect *texture =
                CreateRectangle(perFrameMemory, position, TRANSPARENCY(1.0f), 0.5f, 0.5f);
            texture->bitmap = node;
            texture->bitmapID = node->bitmapID;
            PushRenderGroupRectInfo(gameMetadata->perFrameRenderGroupUI, texture, skipFilter);

            if (selectRectUI && gameMetadata->mouseInfo.mouseType == LEFT_SINGLE_CLICK)
            {
                if (ContainsPoint(texture,
                            V2(ScreenSpaceToNormalizedDeviceSpace(
                                    gameMetadata->mouseInfo.leftScreenCoordinates[0],
                                    gameMetadata->screenResolution))))
                {
                    gameMetadata->editMode.selectedRect->bitmap = texture->bitmap;
                    gameMetadata->editMode.selectedRect->bitmapID = texture->bitmapID;
                }
            }

            position.y -= 0.5f;
        }
    }

    if (!buttonUIFound || !selectRectUI)
    {
        AABB range = DetermineAABBRangeBasedOnMouse(gameMetadata);

        switch(gameMetadata->mouseInfo.mouseType)
        {
            case LEFT_SINGLE_CLICK:
            case LEFT_DRAG_CLICK:
                {
                    Rect *permanentRect =
                        CreateMinimalRectInfo(&gameMetadata->reservedMemory, COLOR_BLUE_TRANSPARENT, &range);
                    permanentRect->type = COLLISION;
                    permanentRect->bitmap = FindBitmap(&gameMetadata->bitmapSentinelNode, "box");
                    permanentRect->bitmapID = permanentRect->bitmap->bitmapID;
                    permanentRect->renderLayer = FRONT_STATIC;
                    PushBack(&gameMetadata->rectManager->NonTraversable.rda, permanentRect);

                    SetAABB(&gameMetadata->rectManager->NonTraversable);
                }
                break;
            case LEFT_MOUSE_DRAG:
                AddDebugRect(gameMetadata, &range, COLOR_RED_TRANSPARENT);
                break;
            case RIGHT_SINGLE_CLICK:
                {
                    /* Find new selected rect to modify */
                    Rect **arr = GetRectsWithInRange(gameMetadata->sm, &range);
                    gameMetadata->editMode.selectedRect = nullptr;
                    for(memory_index i = 0; i < ARRAY_LIST_SIZE(arr); i++)
                    {
                        Rect *rect = arr[i];
                        if (ContainsPoint(rect, range.center))
                        {
                            gameMetadata->editMode.selectedRect = rect;
                            break;
                        }
                    }
                }
                break;
            case MOUSE_WHEEL_UP:
            case MOUSE_WHEEL_DOWN:
                DoCameraUpdate(gameMetadata->mouseInfo.mouseType, gameMetadata->camera);
                break;
        }
    }

    switch(gameMetadata->mouseInfo.mouseType)
    {
        case LEFT_SINGLE_CLICK:
        case LEFT_DRAG_CLICK:
        case RIGHT_SINGLE_CLICK:
        case RIGHT_DRAG_CLICK:
        case MOUSE_WHEEL_DOWN:
        case MOUSE_WHEEL_UP:
            gameMetadata->mouseInfo.mouseType = DO_NOT_USE_ANYMORE_MOUSE;
            break;
    }
}

void Update(GameMetadata *gameMetadata)
{

    /* update logics and data here */
    /* physics */
    GameTimestep *gameTimestep = gameMetadata->gameTimestep;
    UpdateGameTimestep(gameTimestep);

    UpdateBasedOnEditModeChanges(gameMetadata);

    const GLfloat gravity = -9.81f;
    FOR_EACH_HASH_KEY_VAL_BEGIN(HashEntityNPC, hashKeyVal, gameMetadata->hashEntityNPC)
    {
        UpdateNPCMovement(hashKeyVal->val, gameMetadata, gameTimestep->dt);
    }
    FOR_EACH_HASH_KEY_VAL_END();

    /* Update entities */
    UpdateEntities(gameMetadata, gameTimestep, gameMetadata->hitBoxes, gameMetadata->hurtBoxes, gameMetadata->perFrameRenderGroup);
}

#endif
