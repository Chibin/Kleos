#ifndef __SCENE_NODE__
#define __SCENE_NODE__

#include "scene_node.h"

SceneNode *CreateSceneNode(GameMemory *gm, AABB *aabb)
{
    SceneNode *result = (SceneNode *)AllocateMemory(gm, sizeof(SceneNode));
    memset(result, 0, sizeof(SceneNode));
    result->northWest = nullptr;
    result->northEast = nullptr;
    result->southWest = nullptr;
    result->southEast = nullptr;

    result->aabb = *aabb;

    return result;
}

void GetMinMax(SceneNode *sn, MinMax *o_minMax)
{
    GetMinMax(sn->rect, o_minMax);
}

b32 TestAABBAABB(SceneNode *sn, AABB *range)
{
    MinMax rangeMinMax = {};
    MinMax snMinMax = {};
    GetMinMax(range, &rangeMinMax);
    GetMinMax(sn, &snMinMax);

    return TestAABBAABB(&snMinMax, &rangeMinMax);
}

void SubDivide(SceneManager *sm, SceneNode *sn)
{

    AABB aabbNorthWest = {};
    AABB aabbNorthEast = {};
    AABB aabbSouthWest = {};
    AABB aabbSouthEast = {};

    aabbNorthWest.halfDim =
    aabbNorthEast.halfDim =
    aabbSouthWest.halfDim =
    aabbSouthEast.halfDim = sn->aabb.halfDim * 0.5f;

    aabbNorthWest.center = sn->aabb.center + v2{-aabbNorthWest.halfDim.x, aabbNorthWest.halfDim.y};
    aabbNorthEast.center = sn->aabb.center + v2{aabbNorthWest.halfDim.x, aabbNorthWest.halfDim.y};
    aabbSouthWest.center = sn->aabb.center + v2{-aabbNorthWest.halfDim.x, -aabbNorthWest.halfDim.y};
    aabbSouthEast.center = sn->aabb.center + v2{aabbNorthWest.halfDim.x, -aabbNorthWest.halfDim.y};

    sn->northWest = CreateSceneNode(sm->perFrameMemory, &aabbNorthWest);
    sn->northEast = CreateSceneNode(sm->perFrameMemory, &aabbNorthEast);
    sn->southWest = CreateSceneNode(sm->perFrameMemory, &aabbSouthWest);
    sn->southEast = CreateSceneNode(sm->perFrameMemory, &aabbSouthEast);
}

b32 InsertToScenePartition(SceneManager *sm, SceneNode *sn, Rect *rect)
{
    //dimension of each partition
    //insert to the specific partition
    //generate an ID for each specific dimension
    //
    ASSERT(sm->perFrameMemory != nullptr);
    AABB temp = RectToAABB(rect);
    if(TestAABBAABB(&sn->aabb, &temp) == false)
    {
        return false;
    }

    if (sn->rect == nullptr)
    {
#if 0
        AddDebugRect(sm->gameMetadata, &sn->aabb, v4{Randf32(255)/255.0f, Randf32(255)/255.0f, Randf32(255)/255.0f, 1.0f});
#endif
        sn->rect = rect;
        return true;
    }

    if(sn->northWest == nullptr)
    {
        SubDivide(sm, sn);
    }

#if 0
    AddDebugRect(sm->gameMetadata, &sn->northWest->aabb, COLOR_BLUE);
    AddDebugRect(sm->gameMetadata, &sn->northEast->aabb, COLOR_GREEN);
    AddDebugRect(sm->gameMetadata, &sn->southWest->aabb, COLOR_YELLOW);
    AddDebugRect(sm->gameMetadata, &sn->southEast->aabb, COLOR_RED);
#endif

    if(InsertToScenePartition(sm, sn->northWest, rect))
    {
        return true;
    }
    if(InsertToScenePartition(sm, sn->northEast, rect))
    {
        return true;
    }
    if(InsertToScenePartition(sm, sn->southWest, rect))
    {
        return true;
    }
    if(InsertToScenePartition(sm, sn->southEast, rect))
    {
        return true;
    }

    return false;
}

void CreateScenePartition(SceneManager *sm, RectStorage *rs)
{
    sm->aabb = MinMaxToSquareAABB(&rs->aabbMinMax);

#if 0
    AddDebugRect(sm->gameMetadata, &sm->aabb, COLOR_YELLOW);
#endif

    sm->rootSceneNode = CreateSceneNode(sm->perFrameMemory, &sm->aabb);

    for(memory_index i = 0; i < rs->rda.size; i++)
    {
        InsertToScenePartition(sm, sm->rootSceneNode, rs->rda.rects[i]);
    }
}

void QueryRange(SceneManager *sm, SceneNode *sn, Rect ***arr, AABB *range)
{
    if (sn == nullptr)
    {
        return;
    }

    if (TestAABBAABB(&sn->aabb, range) && sn->rect != nullptr)
    {
        ARRAY_PUSH(Rect **, sm->perFrameMemory, *arr, sn->rect);
    }

    QueryRange(sm, sn->northWest, arr, range);
    QueryRange(sm, sn->northEast, arr, range);
    QueryRange(sm, sn->southWest, arr, range);
    QueryRange(sm, sn->southEast, arr, range);

}

Rect **GetRectsWithInRange(SceneManager *sm, AABB *range)
{
    Rect **arr = nullptr;
    ARRAY_CREATE(Rect **, sm->perFrameMemory, arr);

    QueryRange(sm, sm->rootSceneNode, &arr, range);

    return arr;
}

#endif
