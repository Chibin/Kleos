#ifndef __RENDER_GROUP__
#define __RENDER_GROUP__

#include "rectangle.h"

struct RenderGroup
{
    GameMemory vertexMemory;
    memory_index rectCount;
};

inline void ClearUsedRenderGroup(RenderGroup *rg)
{
    rg->rectCount = 0;
    ClearMemoryUsed(&rg->vertexMemory);
};

inline void PushRect(RenderGroup *rg, Rect *rect)
{
    PushRect(&rg->vertexMemory, rect);
    rg->rectCount++;
}

#endif
