#include "scene_node.h"

v2 V2(glm::vec3 A)
{
    v2 result;
    result.x = A.x;
    result.y = A.y;
    return result;

}

v3 V3(glm::vec3 A)
{
    v3 result;
    result.x = A.x;
    result.y = A.y;
    result.z = A.z;
    return result;

}
Rect *CreateMinimalRectInfo(GameMemory *gm, v4 color, v2 min, v2 max)
{
    v2 dim = max - min;
    v2 xyPosition = min + (max - min) / 2.0f;
    v3 xyzPosition = {xyPosition.x, xyPosition.y, 0};
    Rect *r =  CreateRectangle(gm, xyzPosition, color, dim);
    return r;
}

Rect *CreateMinimalRectInfo(GameMemory *gm, v2 min, v2 max)
{
    return CreateMinimalRectInfo(gm, COLOR_RED, min, max);
}
/* I have no idea where to put these functions -- place holder area */
void AddDebugRect(GameMetadata *gm, Rect *rect)
{
    GameMemory *perFrameMemory = &gm->temporaryMemory;
    Rect *newRect = CreateMinimalRectInfo(perFrameMemory, rect->min, rect->max);

    newRect->bitmap = &gm->whiteBitmap;
    newRect->bitmapID = gm->whiteBitmap.bitmapID;
    newRect->renderLayer = DEBUG;

    PushBack(gm->rdaDebug, newRect);
}

void AddDebugRect(GameMetadata *gm, MinMax *minMax, v4 color)
{
    GameMemory *perFrameMemory = &gm->temporaryMemory;
    Rect *newRect = CreateMinimalRectInfo(perFrameMemory, color, minMax->min, minMax->max);

    newRect->bitmap = &gm->whiteBitmap;
    newRect->bitmapID = gm->whiteBitmap.bitmapID;
    newRect->renderLayer = DEBUG;

    PushBack(gm->rdaDebug, newRect);
}

void AddDebugRect(GameMetadata *gm, AABB *aabb, v4 color)
{
    MinMax minMax = {};
    GetMinMax(aabb, &minMax);
    AddDebugRect(gm, &minMax, color);
}

void AddDebugRect(GameMetadata *gm, FrameState *frameState, v3 position, v4 color)
{
    GameMemory *perFrameMemory = &gm->temporaryMemory;
    Rect *newRect = CreateFrameRect(perFrameMemory, frameState, position, color);

    newRect->bitmap = &gm->whiteBitmap;
    newRect->bitmapID = gm->whiteBitmap.bitmapID;
    newRect->renderLayer = DEBUG;

    PushBack(gm->rdaDebug, newRect);
}

struct SceneManager
{
    /* Determines the legnth of each square to partition */
    f32 unitLength;
    SceneNode *rootSceneNode;
    AABB aabb;

    GameMemory *perFrameMemory;
    GameMetadata *gameMetadata;
};

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

AABB MinMaxToSquareAABB(MinMax *minMax)
{
    AABB result = {};

    v2 dim = (minMax->max - minMax->min);
    v2 center = minMax->max - dim * 0.5f;

    v2 squareDim = {};
    squareDim.x = dim.x < dim.y ? dim.y : dim.x;
    squareDim.y = dim.y < dim.x ? dim.x : dim.y;

    result.center = center;
    result.halfDim = squareDim * 0.5f;

    return result;
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

AABB RectToAABB(Rect *rect)
{
    AABB result = {};
    result.center = V2(rect->basePosition);
    result.halfDim = v2{rect->width, rect->height} * 0.5f;
    return result;
}

f32 Randf32(u32 range)
{
    u64 randomNumber = __rdtsc();                                                            \
    u32 mod = randomNumber % range;
    return (f32)mod;
}

b32 InsertToScenePartition(SceneManager *sm, SceneNode *sn, Rect *rect)
{
    //dimension of each partition
    //insert to the specific partition
    //generate an ID for each specific dimension
    //
    ASSERT(sm->perFrameMemory != nullptr);

    if(TestAABBAABB(&sn->aabb, &RectToAABB(rect)) == false)
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

SceneNode *GetObjectsAt(SceneManager *sm, v2 location)
{
    return sm->rootSceneNode;
}

enum ArrayListType
{
    AL_TYPE_SCENE_NODE = 0,
    AL_TYPE_RECT,
    AL_TYPE_SCENE_NODE_PTR,
    AL_TYPE_RECT_PTR,
};

#define DEFAULT_ARRAY_LIST_SIZE 100

struct ArrayList
{
    u32 size;
    memory_index allocatedSize;
    u8 *pData;
    u8 **ppData;

    /* Since this is a "generic" dynamic array, we need to make sure that
     * whatever we insert is always the same size.
     */
    memory_index sizeOfType;
    /* This is an extra check to make sure the user knows what they're doing*/
    ArrayListType type;
};

ArrayList *CreateArrayList(GameMemory *gm, ArrayListType type, memory_index sizeOfType)
{
    ASSERT(sizeOfType != 0);

    ArrayList *arrayList = (ArrayList *)AllocateMemory(gm, sizeof(ArrayList));
    memset(arrayList, 0, sizeof(ArrayList));

    arrayList->type = type;
    arrayList->sizeOfType = sizeOfType;
    arrayList->allocatedSize = sizeOfType * DEFAULT_ARRAY_LIST_SIZE;
    arrayList->pData = nullptr;
    arrayList->ppData = nullptr;

    switch(type)
    {
        case AL_TYPE_SCENE_NODE:
            ASSERT(sizeOfType == sizeof(SceneNode));
            arrayList->pData =
                (u8 *)AllocateMemory(gm, arrayList->allocatedSize);
            break;
        case AL_TYPE_SCENE_NODE_PTR:
            ASSERT(sizeOfType == sizeof(SceneNode *));
            arrayList->ppData =
                (u8 **)AllocateMemory(gm, arrayList->allocatedSize);
            break;
        case AL_TYPE_RECT_PTR:
            ASSERT(sizeOfType == sizeof(Rect *));
            arrayList->ppData =
                (u8 **)AllocateMemory(gm, arrayList->allocatedSize);
            break;
        default:
            ASSERT(!"I Shouldn't get here!");
    }

    return arrayList;
}

void PushBack(GameMemory *gm, ArrayList *al, u8 *data, ArrayListType type)
{
    f32 expansionRate = 0.5f;

    if (al->allocatedSize <= al->size * al->sizeOfType)
    {
        uint32 newTotalSpace =
            SafeCastToU32(al->allocatedSize + SafeCastToU32(al->size * expansionRate * al->sizeOfType));

        ASSERT(newTotalSpace > al->allocatedSize);

        u8 *tmp = *al->ppData;
        u32 oldSize = al->size;

        *al->ppData = (u8 *)AllocateMemory(gm, newTotalSpace);
        memset(*al->ppData, 0, newTotalSpace);
        memcpy(*al->ppData, tmp, al->sizeOfType * oldSize);

#if 0
        /* TODO: Need to verify this */
        free(tmp);
#endif

        al->allocatedSize = newTotalSpace;
    }

    memory_index offset = al->size * al->sizeOfType;
    switch(type)
    {
        case AL_TYPE_SCENE_NODE:
            ASSERT(al->sizeOfType == sizeof(SceneNode));
            //al->pData + offset = data;
            break;
        case AL_TYPE_SCENE_NODE_PTR:
            ASSERT(al->sizeOfType == sizeof(SceneNode *));
            *(al->ppData + offset) = data;
            break;
        case AL_TYPE_RECT_PTR:
            ASSERT(al->sizeOfType == sizeof(Rect *));
            *(al->ppData + offset) = data;
            break;
        default:
            ASSERT(!"I Shouldn't get here!");
    }
    al->size++;
}

void QueryRange(SceneManager *sm, SceneNode *sn, ArrayList *al, AABB *range)
{
    if (sn == nullptr)
    {
        return;
    }

    if (TestAABBAABB(&sn->aabb, range) && sn->rect != nullptr)
    {
#if 0
        AddDebugRect(sm->gameMetadata, &sn->aabb, v4{1.5f, 1.5f, 0.25f, 1.0f});
#endif
        PushBack(sm->perFrameMemory, al, (u8 *)sn->rect, AL_TYPE_RECT_PTR);
    }

    QueryRange(sm, sn->northWest, al, range);
    QueryRange(sm, sn->northEast, al, range);
    QueryRange(sm, sn->southWest, al, range);
    QueryRange(sm, sn->southEast, al, range);
}

ArrayList *GetRectsWithInRange(SceneManager *sm, AABB *range)
{
    ArrayList *al =
        CreateArrayList(sm->perFrameMemory, AL_TYPE_RECT_PTR, sizeof(Rect *));

    QueryRange(sm, sm->rootSceneNode, al, range);
    return al;
}
