#include "game_time.h"
#include "sdl_common.h"

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
}

void UpdateGameTimestep(GameTimestep *gt)
{
    gt->prevTime = gt->latestTime;
    gt->latestTime = SDL_GetTicks();

    static int counter = 0;
    /* Hardcode timestep for now */
    if (counter > 1)
    {
        gt->deltaTime = 5;
        gt->dt = static_cast<real32>(5) / static_cast<real32>(1000.0f);
        counter = 0;
    }
    counter++;
    // gt->deltaTime = gt->latestTime - gt->prevTime;
}
