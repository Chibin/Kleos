#ifndef __PARTICLE_H__
#define __PARTICLE_H__
#include "rectangle.h"
#include "render_group.h"

struct Particle
{
    s32 TTL;
    u32 maxTTL;
    Rect rect;

    v3 positionOffset;
    v3 vel;
    v3 acc;
};

struct ParticleSystem
{
    Particle *particles;
    memory_index particleCount;
};
#endif
