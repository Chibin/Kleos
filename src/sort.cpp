/*
 * MERGE SORT
 */

void MergeRGRI(RenderGroup *rg, GameMemory *rectMemoryTemp, memory_index left, memory_index mid, memory_index right);

void SortRGRI(RenderGroup *rg, GameMemory *rectMemoryTemp, memory_index left, memory_index right)
{
    if ( left < right )
    {
        memory_index mid = (left + right) / 2;

        SortRGRI(rg, rectMemoryTemp, left, mid);
        SortRGRI(rg, rectMemoryTemp, mid + 1, right);
        MergeRGRI(rg, rectMemoryTemp, left, mid, right);
    }
}

void MergeRGRI(RenderGroup *rg, GameMemory *rectMemoryTemp,
        memory_index left, memory_index mid, memory_index right)
{
    /* sorty by highest to lowest render layer */

    memory_index iLeft = left;
    memory_index iRight = mid + 1;

    for (memory_index i = left; i <= right; i++)
    {
        Rect *r = (Rect *)rg->rectMemory.base + i;
        Rect *temp = (Rect *)rectMemoryTemp->base + i;
        *temp = *r;
    }

    memory_index i = left;
    while(i <= right)
    {
        Rect *leftRect = (Rect *)rectMemoryTemp->base + iLeft;
        Rect *rightRect = (Rect *)rectMemoryTemp->base + iRight;
        Rect *r = (Rect *)rg->rectMemory.base + i;

        if (leftRect->renderLayer < rightRect->renderLayer && iRight <= right)
        {
            *r = *rightRect;
            iRight++;
        }
        else if( iLeft <= mid )
        {
            *r = *leftRect;
            iLeft++;
        }
        else if (iRight <= right)
        {
            *r = *rightRect;
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
    RenderGroup *rg;
    GameMemory *rectMemoryTemp;
    memory_index left;
    memory_index right;
};

int ThreadSortRGRI(void *data)
{
    SortData *sortData = (SortData *)data;

    SortRGRI(sortData->rg, sortData->rectMemoryTemp, sortData->left, sortData->right);

    return 0;
}

void MergeSortRenderGroupRectInfo(RenderGroup *rg, GameMemory *perFrameMemory)
{
    /* Render Group Rect Info - RGRI*/
    GameMemory rectMemoryTemp = {};
    u32 rectMemoryBlockSize = sizeof(Rect) * SafeCastToU32(rg->rectEntityCount);
    rectMemoryTemp.base = (u8 *)AllocateMemory(perFrameMemory, rectMemoryBlockSize);
    rectMemoryTemp.maxSize = rectMemoryBlockSize;

#if 1
    SortRGRI(rg, &rectMemoryTemp, 0, rg->rectEntityCount);
#else

    SDL_Thread *leftThread;
    SDL_Thread *rightThread;
    int         threadReturnValue;
    memory_index mid = (0 + rg->rectEntityCount) / 2;

    SortData sortDataLeft = {};
    sortDataLeft.rg = rg;
    sortDataLeft.rectMemoryTemp = &rectMemoryTemp;
    sortDataLeft.left = 0;
    sortDataLeft.right = mid;

    SortData sortDataRight = {};
    sortDataRight.rg = rg;
    sortDataRight.rectMemoryTemp = &rectMemoryTemp;
    sortDataRight.left = mid + 1;
    sortDataRight.right = rg->rectEntityCount;

    leftThread = SDL_CreateThread(ThreadSortRGRI, "SortRGRI", (void *)&sortDataLeft);
    rightThread = SDL_CreateThread(ThreadSortRGRI, "SortRGRI", (void *)&sortDataRight);

    ASSERT(leftThread);
    ASSERT(rightThread);

    SDL_WaitThread(leftThread, &threadReturnValue);
    ASSERT(threadReturnValue == 0);
    SDL_WaitThread(rightThread, &threadReturnValue);
    ASSERT(threadReturnValue == 0);

    MergeRGRI(rg, &rectMemoryTemp, 0, mid, rg->rectEntityCount);
#endif

#if 1
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
    /* TODO: Make sure that the threads are properly shut-down when the game
     * closes.
     * Right now, it will show as a crash once you close the game. It might be
     * related to the thread not properly closing?*/

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
