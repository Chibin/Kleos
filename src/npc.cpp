#ifndef __NPC__
#define __NPC__

struct Movement {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 acceleration;
};

struct NPC
{
    /* movement -/+ dim == min / max */
    Movement movement;
    Bitmap *bitmap;
    v2 dim;

    Animation2D *spriteAnimation;
    FrameState frameState;
    Direction frameDirection;

    RenderLayer renderLayer;
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

    /*
       RectUVCoords *frameCoords;
       u32 totalFrames;
       f32 timePerFrame;
       f32 frameTTL;
       RectUVCoords *currentFrame;
       memory_index currentFrameIndex;
       Direction direction;
    */

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
    UpdateFrameDirection(npc->spriteAnimation, npc->frameDirection);
}

void UpdatePositionBasedOnCollission(NPC *npc, RectManager *rectManager, f32 gravity, f32 dt)
{

    //      if (IntersectionAABB(rect, v2{npc->movement.position.x, npc->movement.position.y},
    //      rayDirection))
    v2 center = {0.5f * npc->dim.width, 0.5f * npc->dim.height};
    /* We only need the min and max from the rect to test for AABB, so we can be a bit reckless here. */
    Rect nextUpdate = {};
    nextUpdate.min = v2{npc->movement.position.x + npc->movement.velocity.x * dt,
        npc->movement.position.y + npc->movement.velocity.y * dt + 0.5f * npc->movement.acceleration.y * dt * dt} - center;
    nextUpdate.max = v2{npc->movement.position.x + npc->movement.velocity.x * dt,
        npc->movement.position.y + npc->movement.velocity.y * dt + 0.5f * npc->movement.acceleration.y * dt * dt} + center;

    for (int i = 0; i < rectManager->NonTraversable.size; i++)
    {
        /* This will need to happen for all AABB checks */
        Rect *rect = rectManager->NonTraversable.rects[i];

        real32 dX = npc->movement.velocity.x * dt;
        real32 dY = npc->movement.velocity.y + (gravity + npc->movement.acceleration.y) * dt;

        glm::vec3 rayDirection = { dX, dY, 0 };
        rayDirection = glm::normalize(rayDirection);
        rayDirection = glm::vec3(1 / rayDirection.x, 1 / rayDirection.y, 0);

        /* This would be nice to be put into the screen instead of the console
         */
        printf("rect X- min: %f max: %f\n", rect->min.x, rect->max.x);
        printf("rect Y- min: %f max: %f\n", rect->min.y, rect->max.y);

        if (rect->type == COLLISION && TestAABBAABB(rect, &nextUpdate))
        {
            /* how to differentiate between x and y axis? */
            printf("I hit something!!!!!!!\n");
            npc->movement.position.y = rect->max.y + npc->dim.height * 0.5f;
            npc->movement.velocity.y = 0;
            npc->movement.acceleration.y = 0;
            npc->movement.position.x += npc->movement.velocity.x * dt;
        }
    }

    /* TODO: There is a bug here. We're not properly updating the position
     * based on the collisions
     */
    npc->movement.acceleration.y += gravity;
    npc->movement.velocity.y += npc->movement.acceleration.y * dt;
    npc->movement.position.y += npc->movement.velocity.y * dt;
    npc->movement.position.x += npc->movement.velocity.x * dt;

}
#endif
