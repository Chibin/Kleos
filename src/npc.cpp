#ifndef __NPC__
#define __NPC__

struct Movement {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

enum MovementType
{
    X_MOVEMENT,
    Y_MOVEMENT,
    XY_MOVEMENT
};

enum MovementPattern
{
    WAYPOINT,
    UNI_DIRECTIONAL,
    BI_DIRECTIONAL,
};

struct NPC
{
    /* movement -/+ dim == min / max */
    Movement movement;
    Bitmap *bitmap;
    v2 dim;

    Animation2D *spriteAnimation;
    FrameState frameState;

    RenderLayer renderLayer;

    MovementType movementType;
    MovementPattern movementPattern;

    Direction direction;
};

Rect *CreateMinimalRectInfo(GameMemory *gm, NPC *npc)
{
    Rect *r =  CreateRectangle(gm, npc->movement.position, COLOR_WHITE, npc->dim);

    r->bitmap = npc->bitmap;
    r->renderLayer = npc->renderLayer;
    return r;
};

RectUVCoords *CopyUVCoords(GameMemory *gm, RectUVCoords *ruvc, memory_index size)
{
    RectUVCoords *uvCoordsCopy = (RectUVCoords *)AllocateMemory(gm, sizeof(RectUVCoords) * size);
    memcpy(uvCoordsCopy, ruvc, sizeof(RectUVCoords) * size);
    return uvCoordsCopy;
}

Animation2D *CreateCopyOfSpriteAnimationInfo(GameMemory *gm, Animation2D *spriteSource)
{
    Animation2D *spriteCopy = (Animation2D *)AllocateMemory(gm, sizeof(Animation2D));
    memset(spriteCopy, 0, sizeof(Animation2D));

    spriteCopy->frameCoords = CopyUVCoords(gm, spriteSource->frameCoords, spriteSource->totalFrames);
    spriteCopy->totalFrames = spriteSource->totalFrames;
    spriteCopy->timePerFrame = spriteSource->timePerFrame;
    spriteCopy->frameTTL = spriteSource->frameTTL;
    spriteCopy->currentFrame = &spriteCopy->frameCoords[0];

    return spriteCopy;
}

void UpdateNPCAnimation(NPC *npc, Rect *r)
{
    UpdateCurrentFrame(npc->spriteAnimation, 17.6f);
    UpdateUV(r, *npc->spriteAnimation->currentFrame);
    UpdateFrameDirection(npc->spriteAnimation, npc->direction);
}

void UpdatePositionBasedOnCollission(NPC *npc, RectManager *rectManager, f32 gravity, f32 dt)
{

    v2 center = {0.5f * npc->dim.width, 0.5f * npc->dim.height};
    /* We only need the min and max from the rect to test for AABB, so we can be a bit reckless here. */
    Rect nextUpdate = {};
    nextUpdate.min =
        V2(npc->movement.position) + V2(npc->movement.velocity) * dt + 0.5f * V2(npc->movement.acceleration) * dt * dt - center;
    nextUpdate.max =
        V2(npc->movement.position) + V2(npc->movement.velocity) * dt + 0.5f * V2(npc->movement.acceleration) * dt * dt + center;

    npc->movement.acceleration.y += gravity;
    for (int i = 0; i < rectManager->NonTraversable.size; i++)
    {
        /* This will need to happen for all AABB checks */
        Rect *rect = rectManager->NonTraversable.rects[i];
        if (rect->type == COLLISION && TestAABBAABB(rect, &nextUpdate))
        {
            /* XXX: This is the part where we should figure out the direction of the collision */
            npc->movement.position.y = rect->max.y + npc->dim.height * 0.5f;
            npc->movement.velocity.y = 0;
            npc->movement.acceleration.y = gravity;

            break;
        }
    }

    /* TODO: There is a bug here. We're not properly updating the position
     * based on the collisions
     */
    npc->movement.position += npc->movement.velocity * dt + 0.5f * npc->movement.acceleration * dt * dt;
    npc->movement.velocity.y += npc->movement.acceleration.y *dt;

    /* Apply "friction" */
    npc->movement.velocity.x = 0;
}

void NPCMoveRight(NPC *npc)
{
    npc->direction = RIGHT;
    npc->movement.velocity.x += 1.0f;
}

void NPCMoveLeft(NPC *npc)
{
    npc->direction = LEFT;
    npc->movement.velocity.x -= 1.0f;
}

b32 CanMoveTheSameDirection(NPC *npc, GameMetadata *gameMetadata, RectManager *rectManager, f32 gravity, f32 dt)
{
    v2 center = {0.5f * npc->dim.width, 0.5f * npc->dim.height};
    /* We only need the min and max from the rect to test for AABB, so we can be a bit reckless here. */
    GameMemory *perFrameMemory = &gameMetadata->temporaryMemory;
    Rect nextUpdate = {};

    nextUpdate.min =
        V2(npc->movement.position) + V2(npc->movement.velocity) * dt + 0.5f * V2(npc->movement.acceleration) * dt * dt - center;
    nextUpdate.max =
        V2(npc->movement.position) + V2(npc->movement.velocity) * dt + 0.5f * V2(npc->movement.acceleration) * dt * dt + center;

    switch(npc->direction)
    {
        case LEFT:
            /* XXX: make it such that the NPC turns early*/
            nextUpdate.max.x = 0.01f + nextUpdate.min.x;
            AddDebugRect(gameMetadata, &nextUpdate);

            for (int i = 0; i < rectManager->NonTraversable.size; i++)
            {
                if(TestAABBAABB(rectManager->NonTraversable.rects[i], &nextUpdate))
                {
                    return true;
                }
            }
            break;
        case RIGHT:
            /* XXX: Hack to make it such that the nextUpdate rect will be a
             * small sliver of the actual rect width.
             * This will make the NPC turn earlier instad of hitting the edge
             * of the platform with the last pixel.
             */
            nextUpdate.min.x = nextUpdate.max.x - 0.01f;
            AddDebugRect(gameMetadata, &nextUpdate);

            for (int i = 0; i < rectManager->NonTraversable.size; i++)
            {
                if(TestAABBAABB(rectManager->NonTraversable.rects[i], &nextUpdate))
                {
                    return true;
                }
            }
            break;
        default:
            ASSERT(!"I SHOUDLN'T GET HERE");
    }

    return false;
}

void SwitchDirection(NPC *npc)
{
    switch(npc->direction)
    {
        case LEFT:
            npc->direction = RIGHT;
            break;
        case RIGHT:
            npc->direction = LEFT;
            break;
        default:
            ASSERT(!"I shoudln't get here");
            break;
    }
}

void MoveUniDirectional(NPC *npc, GameMetadata *gm, RectManager *rectManager, f32 gravity, f32 dt)
{
    switch(npc->direction)
    {
        case LEFT:
            if (CanMoveTheSameDirection(npc, gm, rectManager, gravity, dt) == false)
            {
                npc->direction = RIGHT;
            }
            break;
        case RIGHT:
            if (CanMoveTheSameDirection(npc, gm, rectManager, gravity, dt) == false)
            {
                npc->direction = LEFT;
            }
            break;
        default:
            ASSERT(!"I shoudln't get here");
            break;
    }

    switch(npc->direction)
    {
        case LEFT:
            NPCMoveLeft(npc);
            break;
        case RIGHT:
            NPCMoveRight(npc);
            break;
        default:
            ASSERT(!"I shoudln't get here");
            break;
    }
}

void MoveBiDirectional(NPC *npc)
{

}

void NPCMoveX(NPC *npc, GameMetadata *gm, RectManager *rectManager, f32 gravity, f32 dt)
{
    switch(npc->movementPattern)
    {
        case UNI_DIRECTIONAL:
        {
            MoveUniDirectional(npc, gm, rectManager, gravity, dt);
            break;
        }
        case BI_DIRECTIONAL:
        {
            MoveBiDirectional(npc);
            break;
        }
        default:
            ASSERT(!"I SHOULDN'T GET HERE");
    }
}

void NPCMove(NPC *npc, GameMetadata *gm, RectManager *rectManager, f32 gravity, f32 dt)
{
    /* This is where the NPC will decide where to move.
     * For now, let's make the NPC move left and right.*/

    // Check desired location
    switch (npc->movementType)
    {
        case X_MOVEMENT:
            NPCMoveX(npc, gm, rectManager, gravity, dt);
            //or
            //NPCMoveRight(npc);
            break;
        case Y_MOVEMENT:
            break;
        case XY_MOVEMENT:
            break;
        default:
            ASSERT(!"I SHOULDN'T GET HERE");
    }
    //NPCMoveRight(npc);
    /*
    NPCMoveUp();
    NPCMoveDown();
    */
}

void UpdateNPCMovement(NPC *npc, GameMetadata *gm, RectManager *rectManager, f32 gravity, f32 dt)
{
    NPCMove(npc, gm, rectManager, gravity, dt);
}
#endif