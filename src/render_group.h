#ifndef __RENDER_GROUP__
#define __RENDER_GROUP__

#include "rectangle.h"

struct RenderGroup
{
    GameMemory vertexMemory;
    GameMemory rectMemory;

    memory_index rectCount;

    memory_index rectEntityCount;
};

inline void ClearUsedVertexRenderGroup(RenderGroup *rg)
{
    rg->rectCount = 0;
    ClearMemoryUsed(&rg->vertexMemory);
};

inline void ClearUsedRenderGroup(RenderGroup *rg)
{
    rg->rectCount = 0;
    ClearMemoryUsed(&rg->vertexMemory);
};

inline void PushRectVertex(RenderGroup *rg, Rect *rect)
{
    PushRectVertex(&rg->vertexMemory, rect);
    rg->rectCount++;
}

inline void PushRectInfo(RenderGroup *rg, Rect *rect)
{
    PushRectInfo(&rg->rectMemory, rect);
    rg->rectEntityCount++;
}
#endif
