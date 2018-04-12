#pragma once

#include "math.h"

struct GameTimestep
{
    uint32 prevTime;
    uint32 latestTime;
    uint32 deltaTime;
    real32 dt;
};

void PauseGameTimestep();
void ResetGameTimestep(GameTimestep *gt);
void UpdateGameTimestep(GameTimestep *gt);
