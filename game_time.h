#pragma once

#include "logger.h"
#include "math.h"

#if WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#define _CAT(x, y) x##y
#define CAT(x, y) _CAT(x, y)

#define START_DEBUG_TIMING()              \
    GameTimestep gameTimestep_local = {}; \
    ResetGameTimestep(&gameTimestep_local);

#define PRINT_ELAPSED_TIME(x)                                                                 \
    GameTimestep *gt = x;                                                                     \
    u64 endCounter = SDL_GetPerformanceCounter();                                             \
    u64 counterElapsed = endCounter - gt->lastCounter;                                        \
    f64 MSPerFrame = (((1000.0f * (real64)counterElapsed) / (real64)gt->perfCountFrequency)); \
    f64 FPS = (real64)gt->perfCountFrequency / (real64)counterElapsed;                        \
    gt->lastCounter = endCounter;                                                             \
    u64 endCycleCount = __rdtsc();                                                            \
    u64 cyclesElapsed = endCycleCount - gt->lastCycleCount;                                   \
    f64 MCPF = ((f64)cyclesElapsed / (1000.0f * 1000.0f));                                    \
    printf("  %.02f ms/f    %.0ff/s    %.02fcycles/f  \n", MSPerFrame, FPS, MCPF);

#define END_DEBUG_TIMING() \
    PRINT_ELAPSED_TIME(&gameTimestep_local);

struct GameTimestep
{
    uint32 prevTime;
    uint32 latestTime;
    uint32 deltaTime;
    real32 dt;

    u64 perfCountFrequency;
    u64 lastCounter;
    u64 endCounter;
    u64 counterElapsed;

    u64 lastCycleCount;
};

void PauseGameTimestep();
void ResetGameTimestep(GameTimestep *gt);
void UpdateGameTimestep(GameTimestep *gt);
