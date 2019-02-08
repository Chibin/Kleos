#ifndef __ASSET__
#define __ASSET__

#define MAX_NUM_CHARS 5 /* u8 -> 3 characters, +1 for colon, +1 for \n */

struct PixelCoords
{
    v2i pixel[4];
};

struct FrameCycle
{
    char name[32];
    PixelCoords *frames;
    u8 frameCount;

    RectUVCoords *frameCoords;

    Animation2D *animationInfo;
};

/* XXX: This is probably the incorrect name.
 * It's close to a sprite animation, where all the sprite cycles reside on each
 * FrameAnimation/SpriteAnimation.
 * A single Frame Animation can have one or more frame cycles.
 * Frame cycle being -> attack frame cycle
 *                   -> idle frame cycle
 *                   -> walk frame cycle
 *                   -> etc frame cycle
 */
struct FrameAnimation
{
    char animationName[256];
    u8 animationCount;
    FrameCycle *frameCycles;

    FrameAnimation *next;
    FrameAnimation *prev;
};

enum FrameDataReadState
{
    NAME,
    ANIMATION_COUNT,
    FRAME_STATE_NAME,
    FRAME_COUNT,
    FRAMES,
};

#endif
