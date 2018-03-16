#ifndef __GAME_TIME__
#define __GAME_TIME__

struct GameTimestep {
    uint32 prevTime;
    uint32 latestTime;
    uint32 deltaTime;
};

void PauseGameTimestep()
{

}

void ResetGameTimestep(GameTimestep *gt)
{
    uint32 now = SDL_GetTicks();
    gt->prevTime = now;
    gt->latestTime = now;
    gt->deltaTime = 0;
}


void UpdateGameTimestep(GameTimestep *gt)
{
    gt->prevTime = gt->latestTime;
    gt->latestTime = SDL_GetTicks();

    gt->deltaTime = gt->latestTime - gt->prevTime;
}
#endif
