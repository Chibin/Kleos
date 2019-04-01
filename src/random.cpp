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

void AddDebugRect(GameMetadata *gm, AABB *aabb, v4 color)
{
    MinMax minMax = {};
    GetMinMax(aabb, &minMax);
    GameMemory *perFrameMemory = &gm->temporaryMemory;
    Rect *newRect = CreateMinimalRectInfo(perFrameMemory, color, minMax.min, minMax.max);

    newRect->bitmap = &gm->whiteBitmap;
    newRect->bitmapID = gm->whiteBitmap.bitmapID;
    newRect->renderLayer = DEBUG;

    PushBack(gm->rdaDebug, newRect);
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
    SceneNode *sceneNodes;

    GameMemory *perFrameMemory;
};

SceneNode *CreateSceneNode(GameMemory *gm)
{
    SceneNode *result = (SceneNode *)AllocateMemory(gm, sizeof(SceneNode));
    memset(result, 0, sizeof(SceneNode));
    result->northWest = nullptr;
    result->northEast = nullptr;
    result->southWest = nullptr;
    result->southEast = nullptr;

    return result;
}

void InsertToScenePartition(SceneManager *sm, Rect *rect)
{
    //dimension of each partition
    //insert to the specific partition
    //generate an ID for each specific dimension
    //
    ASSERT(sm->perFrameMemory != nullptr);
    SceneNode *sn = CreateSceneNode(sm->perFrameMemory);
    sn->rect = rect;

    SceneNode *tmp = sm->sceneNodes;
    sm->sceneNodes = sn;
    sm->sceneNodes->northWest = tmp;
}

void CreateScenePartition(SceneManager *sm, RectDynamicArray *rda)
{
    for(memory_index i = 0; i < rda->size; i++)
    {
        InsertToScenePartition(sm, rda->rects[i]);
    }
}

SceneNode *GetObjectsAt(SceneManager *sm, v2 location)
{
    return sm->sceneNodes;
}

enum ArrayListType
{
    AL_TYPE_SCENE_NODE = 0,
    AL_TYPE_SCENE_NODE_PTR,
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
        default:
            ASSERT(!"I Shouldn't get here!");
    }

    return arrayList;
}

void PushBack(GameMemory *gm, ArrayList *al, u8 *data, ArrayListType type)
{
    f32 expansionRate = 0.25;

    if (al->allocatedSize <= al->size)
    {
        uint32 newTotalSpace =
            SafeCastToU32(al->allocatedSize + SafeCastToU32(al->size * expansionRate * al->sizeOfType));

        u8 *tmp = *al->ppData;
        u32 oldSize = al->size;

        *al->ppData = (u8 *)AllocateMemory(gm, newTotalSpace);
        memset(*al->ppData, 0, newTotalSpace);
        memcpy(*al->ppData, tmp, al->sizeOfType * oldSize);

        free(tmp);

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
        default:
            ASSERT(!"I Shouldn't get here!");
    }
    al->size++;
}

ArrayList *GetRectsWithInRange(SceneManager *sm, AABB *range)
{
    ArrayList *al =
        CreateArrayList(sm->perFrameMemory, AL_TYPE_SCENE_NODE_PTR, sizeof(SceneNode *));

    for(SceneNode *sn = sm->sceneNodes; sn != nullptr; sn = sn->northWest)
    {
        if (TestAABBAABB(sn, range))
        {
            PushBack(sm->perFrameMemory, al, (u8 *)sn, AL_TYPE_SCENE_NODE_PTR);
        }
    };

    return al;
}
