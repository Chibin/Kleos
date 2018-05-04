#pragma once

#include "math.h"

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
