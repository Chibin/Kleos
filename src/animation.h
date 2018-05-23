#ifndef __ANIMATION_H__
#define __ANIMATION_H__

struct Animation2D
{
    RectUVCoords *frameCoords;
    u32 totalFrames;
    /* May be have a function pointer or some type of function to determine how
     * long each frames last.
     * Or a waypoint curve?
     */
    f32 timePerFrame;
    f32 frameTTL;
    RectUVCoords *currentFrame;
    memory_index currentFrameIndex;
    Direction direction;
};

#endif
