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
};

Rect *CreateMinimalRectInfo(GameMemory *gm, NPC *npc)
{
    Rect *r =  CreateRectangle(gm, npc->movement.position, COLOR_WHITE, npc->dim);

    r->bitmap = npc->bitmap;
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
#endif
