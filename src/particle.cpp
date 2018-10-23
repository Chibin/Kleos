#ifndef __PARTICLE__
#define __PARTICLE__

#include "particle.h"

void UpdateParticleTransparency(Particle *p)
{
    f32 alpha = (f32)(p->TTL / (f32)p->maxTTL); // NOLINT
    UpdateColors(&p->rect,
                 v4{ p->rect.color.r,
                     p->rect.color.g,
                     p->rect.color.b,
                     alpha}
                );
}

inline void UpdateParticlePosition(Particle *p, f32 dt)
{
    v3 pos = p->positionOffset;
    v3 vel = p->vel;
    v3 acc = p->acc;
    p->positionOffset.x = pos.x + vel.x * dt;
    p->positionOffset.y = pos.y + vel.y * dt + 0.5f * acc.y * dt * dt;
}

inline void UpdateAndGenerateParticleRectInfo(RenderGroup *renderGroup, ParticleSystem *ps, s32 ttl)
{
    for (memory_index i = 0; i < ps->particleCount; i++)
    {
        Particle *particle = &ps->particles[i]; // NOLINT
        particle->TTL -= ttl;

        if (particle->TTL <= 0)
        {
            continue;
        }

        f32 alpha = (f32)(particle->TTL / (f32)particle->maxTTL); // NOLINT
        UpdateColors(&particle->rect,
                v4{ particle->rect.color.r,
                particle->rect.color.g,
                particle->rect.color.b,
                alpha});

        PushRenderGroupRectInfo(renderGroup, &particle->rect);
    }
}
#endif
