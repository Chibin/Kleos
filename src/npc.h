#ifndef __NPC_H__
#define __NPC_H__

#include "movement.h"
struct NPC
{
    /* movement -/+ dim == min / max */
    Movement *movement;
    Bitmap *bitmap;
    v2 dim;

    Animation2D *spriteAnimation;
    FrameState frameState;

    RenderLayer renderLayer;

    MovementType movementType;
    MovementPattern movementPattern;

    Direction direction;
};

#endif
