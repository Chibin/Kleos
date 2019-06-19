#ifndef __STATS__
#define __STATS__

#include "stats.h"

Stat *CreateStat(GameMemory *gameMemory, s32 maxHealth)
{
    Stat *result = (Stat *)AllocateMemory0(gameMemory, sizeof(Stat));
    result->maxHealth = maxHealth;
    result->currentHealth = maxHealth;

    return result;
}
#endif
