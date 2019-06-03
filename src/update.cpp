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

    FOR_EACH_HASH_KEY_VAL_BEGIN(HashEntityMovement, hashKeyVal, gameMetadata->hashEntityMovement)
    {
        Entity *e = hashKeyVal->key;
        Movement *movement = hashKeyVal->val;
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

void UpdateBasedOnEditModeChanges(GameMetadata *gameMetadata)
{
    if (gameMetadata->isEditMode == false)
    {
        return;
    }

    AABB range = {};
    range.halfDim = V2(gameMetadata->leftMouseDrag[0] - gameMetadata->leftMouseDrag[1]) * 0.5f;
    range.center = V2(gameMetadata->leftMouseDrag[0]) - range.halfDim;
    /* sometimes we're going to the negative value when calculating the
     * dimension. This needs to be positive in order to work.
     * We also need to use the negative dimension to calculate two out of the four quadrants.
     */
    range.halfDim = abs(range.halfDim);

    if (gameMetadata->createNewRect)
    {

        gameMetadata->createNewRect = false;

        Rect *permanentRect = CreateMinimalRectInfo(&gameMetadata->reservedMemory, COLOR_BLUE_TRANSPARENT, &range);
        permanentRect->type = COLLISION;
        permanentRect->bitmapID = FindBitmap(&gameMetadata->bitmapSentinelNode, "box")->bitmapID;
        permanentRect->renderLayer = FRONT_STATIC;
        PushBack(&gameMetadata->rectManager->NonTraversable.rda, permanentRect);

        SetAABB(&gameMetadata->rectManager->NonTraversable);
    }

    if (gameMetadata->isLeftButtonReleased == false)
    {
        AddDebugRect(gameMetadata, &range, COLOR_RED_TRANSPARENT);
    }

    if (gameMetadata->willSelectObject)
    {
        gameMetadata->willSelectObject = false;
        range.center = V2(gameMetadata->rightMouseButton);
        f32 arbitraryPadding = 15.0f;
        range.halfDim = range.halfDim + arbitraryPadding;
        Rect **arr = GetRectsWithInRange(gameMetadata->sm, &range);
#if 0
        AddDebugRect(gameMetadata, &range, COLOR_GREEN_TRANSPARENT);
#endif
        gameMetadata->selectedRect = nullptr;
        for(memory_index i = 0; i < ARRAY_LIST_SIZE(arr); i++)
        {
            Rect *rect = arr[i];
            if (ContainsPoint(rect, range.center))
            {
#if 0
                AddDebugRect(gameMetadata, rect, COLOR_YELLOW_TRANSPARENT);
#endif
                gameMetadata->selectedRect = rect;
                break;
            }
        }
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
