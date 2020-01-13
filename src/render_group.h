#ifndef __RENDER_GROUP__
#define __RENDER_GROUP__

#include "rectangle.h"
#include "collision.h"

struct RenderGroup
{
    GameMemory vertexMemory;
    GameMemory rectMemory;

    memory_index rectCount;

    memory_index rectEntityCount;

    MinMax minMax;
};

inline void ClearUsedVertexRenderGroup(RenderGroup *rg)
{
    rg->rectCount = 0;
    ClearMemoryUsed(&rg->vertexMemory);
};

inline void ClearUsedRectInfoRenderGroup(RenderGroup *rg)
{
    rg->rectEntityCount = 0;
    ClearMemoryUsed(&rg->rectMemory);
};

inline void ClearUsedRenderGroup(RenderGroup *rg)
{
    rg->rectCount = 0;
    ClearMemoryUsed(&rg->vertexMemory);
};

inline void PushRenderGroupRectVertex(RenderGroup *rg, Rect *rect)
{
    PushRectVertex(&rg->vertexMemory, rect);
    rg->rectCount++;
}

/*
 * if the filter is skipped, we no longer need to check the bounds
 */
inline void PushRenderGroupRectInfo(RenderGroup *rg, Rect *rect, b32 skipFilter)
{
    if (skipFilter)
    {
        PushRectInfo(&rg->rectMemory, rect);
        rg->rectEntityCount++;
        return;
    }

    MinMax rectMinMax = {rect->min, rect->max};
    if (TestAABBAABB(&rg->minMax, &rectMinMax))
    {
        PushRectInfo(&rg->rectMemory, rect);
        rg->rectEntityCount++;
    }
}

inline RenderGroup *CreateRenderGroup(
        GameMemory *perFrameMemory, u32 numOfPointsPerRect, u16 maxEntityCount, MinMax minMax)
{
    RenderGroup *result = (RenderGroup *)AllocateMemory(perFrameMemory, sizeof(RenderGroup));
    memset(result, 0, sizeof(RenderGroup));
    u32 vertexBlockSize = sizeof(Vertex) * numOfPointsPerRect * maxEntityCount;
    result->vertexMemory.base = (u8 *)AllocateMemory(perFrameMemory, vertexBlockSize);
    result->vertexMemory.maxSize = vertexBlockSize;

    u32 rectMemoryBlockSize = sizeof(Rect) * maxEntityCount;
    result->rectMemory.base = (u8 *)AllocateMemory(perFrameMemory, rectMemoryBlockSize);
    result->rectMemory.maxSize = rectMemoryBlockSize;

    result->minMax = minMax;

    return result;
}
#endif
