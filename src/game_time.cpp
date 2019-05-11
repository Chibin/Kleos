#ifndef __GAME_TIME__
#define __GAME_TIME__

#include "game_time.h"
#include "sdl_common.h"

b32 IsGamePaused(GameTimestep *gt)
{
    return gt->isPaused;
}

void ResetGameTimestep(GameTimestep *gt)
{
    uint32 now = SDL_GetTicks();
    gt->prevTime = now;
    gt->latestTime = now;
    gt->deltaTime = 0;
    gt->dt = 0;
    gt->isPaused = false;

    gt->perfCountFrequency = SDL_GetPerformanceFrequency();
    gt->lastCounter = SDL_GetPerformanceCounter();
    gt->lastCycleCount = __rdtsc();
}

void UpdateGameTimestep(GameTimestep *gt)
{
    if (IsGamePaused(gt))
    {
        gt->deltaTime = 0;
        gt->dt = 0;
        return;
    }

    gt->prevTime = gt->latestTime;
    gt->latestTime = SDL_GetTicks();

    static int counter = 0;
    /* Hardcode timestep for now */
    if (counter > 1)
    {
        gt->deltaTime = 16;
        gt->dt = static_cast<real32>(gt->deltaTime) / static_cast<real32>(1000.0f);
        counter = 0;
    }
    counter++;
    gt->deltaTime = gt->latestTime - gt->prevTime;
}

void CalculateFrameStatistics(
        GameTimestep *gt,
        f64 *MSPerFrame,
        f64 *FPS,
        f64 *MCPF)
{
    u64 endCounter = SDL_GetPerformanceCounter();
    u64 counterElapsed = endCounter - gt->lastCounter;

    *MSPerFrame = (((1000.0f * (real64)counterElapsed) / (real64)gt->perfCountFrequency));
    *FPS = (real64)gt->perfCountFrequency / (real64)counterElapsed;
    gt->lastCounter = endCounter;

    u64 endCycleCount = __rdtsc();
    u64 cyclesElapsed = endCycleCount - gt->lastCycleCount;
    *MCPF = ((f64)cyclesElapsed / (1000.0f * 1000.0f));

    gt->lastCycleCount = endCycleCount;
}
#endif
