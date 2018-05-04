#include "game_time.h"
#include "sdl_common.h"
#if WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

void PauseGameTimestep()
{
}

void ResetGameTimestep(GameTimestep *gt)
{
    uint32 now = SDL_GetTicks();
    gt->prevTime = now;
    gt->latestTime = now;
    gt->deltaTime = 0;
    gt->dt = 0;

    gt->perfCountFrequency = SDL_GetPerformanceFrequency();
    gt->lastCounter = SDL_GetPerformanceCounter();
    gt->lastCycleCount = __rdtsc();
}

void UpdateGameTimestep(GameTimestep *gt)
{
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
