/*
 * MERGE SORT
 */

void MergeRGRI(Rect **result, Rect **rectMemoryTemp, memory_index left, memory_index mid, memory_index right);

void SortRGRI(Rect **result, Rect **rectMemoryTemp, memory_index left, memory_index right)
{
    if ( left < right )
    {
        memory_index mid = (left + right) / 2;

        SortRGRI(result, rectMemoryTemp, left, mid);
        SortRGRI(result, rectMemoryTemp, mid + 1, right);
        MergeRGRI(result, rectMemoryTemp, left, mid, right);
    }
}

void MergeRGRI(Rect **result, Rect **rectMemoryTemp, memory_index left, memory_index mid, memory_index right)
{
    /* sorty by highest to lowest render layer */

    memory_index iLeft = left;
    memory_index iRight = mid + 1;

    for (memory_index i = left; i <= right; i++)
    {
        rectMemoryTemp[i] = result[i];
    }

    memory_index i = left;
    while(i <= right)
    {
        Rect *leftRect = (Rect *)rectMemoryTemp[iLeft];
        Rect *rightRect = (Rect *)rectMemoryTemp[iRight];
        Rect *r = (Rect *)result[i];

        if (iRight <= right && leftRect->renderLayer < rightRect->renderLayer)
        {
            result[i] = rectMemoryTemp[iRight];
            iRight++;
        }
        else if( iLeft <= mid )
        {
            result[i] = rectMemoryTemp[iLeft];
            iLeft++;
        }
        else if (iRight <= right)
        {
            result[i] = rectMemoryTemp[iRight];
            iRight++;
        }
        else
        {
            ASSERT(!"I shouldn't get here\n");
        }

        i++;
    }
}

struct SortData
{
    Rect **result;
    Rect **rectMemoryTemp;
    memory_index left;
    memory_index right;
};

int ThreadSortRGRI(void *data)
{
    SortData *sortData = (SortData *)data;

    /* TODO: FIX ME */
    SortRGRI(sortData->result, sortData->rectMemoryTemp, sortData->left, sortData->right);

    return 0;
}

Rect **MergeSortRenderGroupRectInfo(RenderGroup *rg, GameMemory *perFrameMemory)
{
    /* Render Group Rect Info - RGRI*/
    u32 rectMemoryBlockSize = sizeof(Rect *) * SafeCastToU32(rg->rectEntityCount);

    Rect **ppRectMemoryTemp = (Rect **)AllocateMemory(perFrameMemory, rectMemoryBlockSize);
    Rect **ppOrderedMemory = (Rect **)AllocateMemory(perFrameMemory, rectMemoryBlockSize);

    for (memory_index i = 0; i < rg->rectEntityCount; i++)
    {
        ppOrderedMemory[i] = (Rect *)rg->rectMemory.base + i;
        ASSERT(ppOrderedMemory[i]->bitmap != NULL);
    }

#if 1
    SortRGRI(ppOrderedMemory, ppRectMemoryTemp, 0, rg->rectEntityCount - 1);
#else
    SDL_Thread *leftThread;
    SDL_Thread *rightThread;
    int         threadReturnValue;
    memory_index mid = (0 + rg->rectEntityCount) / 2;

    SortData sortDataLeft = {};
    sortDataLeft.result = ppOrderedMemory;
    sortDataLeft.rectMemoryTemp = ppRectMemoryTemp;
    sortDataLeft.left = 0;
    sortDataLeft.right = mid;

    SortData sortDataRight = {};
    sortDataRight.result = ppOrderedMemory;
    sortDataRight.rectMemoryTemp = ppRectMemoryTemp;
    sortDataRight.left = mid + 1;
    sortDataRight.right = rg->rectEntityCount - 1;

    leftThread = SDL_CreateThread(ThreadSortRGRI, "SortRGRI", (void *)&sortDataLeft);
    rightThread = SDL_CreateThread(ThreadSortRGRI, "SortRGRI", (void *)&sortDataRight);

    ASSERT(leftThread);
    ASSERT(rightThread);

    SDL_WaitThread(leftThread, &threadReturnValue);
    ASSERT(threadReturnValue == 0);
    SDL_WaitThread(rightThread, &threadReturnValue);
    ASSERT(threadReturnValue == 0);

    MergeRGRI(ppOrderedMemory, ppRectMemoryTemp, 0, mid, rg->rectEntityCount - 1);
#endif

#if 1
    /* debug purposes */
    Rect *prevRect = nullptr;

    for (memory_index i = 0; i < rg->rectEntityCount; i++)
    {
        Rect *r = (Rect *)ppOrderedMemory[i];

        if (prevRect)
        {
            ASSERT(prevRect->renderLayer >= r->renderLayer);
        }
        prevRect = r;
    }

#endif
    return ppOrderedMemory;
}

/*
 * QUICK SORT
 */

memory_index PartitionRGRI(RenderGroup *rg, memory_index left,  memory_index right)
{

    memory_index partitionIndex = (left + right) / 2;

    while (left <= right)
    {
        Rect *leftRect = (Rect *)rg->rectMemory.base + left;
        Rect *partRect = (Rect *)rg->rectMemory.base + partitionIndex;
        Rect *rightRect = (Rect *)rg->rectMemory.base + right;

        while(leftRect->renderLayer > partRect->renderLayer)
        {
            left++;
            leftRect = (Rect *)rg->rectMemory.base + left;
        }

        while(rightRect->renderLayer < partRect->renderLayer)
        {
            right--;
            rightRect = (Rect *)rg->rectMemory.base + right;
        }

        if (left <= right)
        {
            Rect tmp = *rightRect;
            *rightRect = *leftRect;
            *leftRect = tmp;

            left++;
            right--;

            leftRect = (Rect *)rg->rectMemory.base + left;
            rightRect = (Rect *)rg->rectMemory.base + right;
        }
    }

    return left;
}

void QuickSortRGRI(RenderGroup *rg, memory_index left, memory_index right)
{
    memory_index index = PartitionRGRI(rg, left, right);

    if (left < index - 1)
    {
        QuickSortRGRI(rg, left, index - 1);
    }
    if (index < right)
    {
        QuickSortRGRI(rg, index, right);
    }
}

void QuickSortRenderGroupRectInfo(RenderGroup *rg)
{
    QuickSortRGRI(rg, 0, rg->rectEntityCount - 1);

#if 0
    /* debug purposes */
    Rect *prevRect = nullptr;

    for (memory_index i = 0; i < rg->rectEntityCount; i++)
    {
        Rect *r = (Rect *)rg->rectMemory.base + i;

        if (prevRect)
        {
            ASSERT(prevRect->renderLayer >= r->renderLayer);
        }
        prevRect = r;
    }
#endif

}

void SortDrawEntities()
{

}
