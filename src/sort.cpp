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

#if 1
    /* debug purposes */
    if (left == 0)
    {
        Rect *prevRect = nullptr;

        for (memory_index i = 0; i < right; i++)
        {
            Rect *r = (Rect *)rg->rectMemory.base + i;

            if (prevRect)
            {
                ASSERT(prevRect->renderLayer >= r->renderLayer);
            }
            prevRect = r;
        }

    }
#endif

}

void MergeSortRenderGroupRectInfo(RenderGroup *rg, GameMemory *perFrameMemory)
{
    /* Render Group Rect Info - RGRI*/
    GameMemory rectMemoryTemp = {};
    u32 rectMemoryBlockSize = sizeof(Rect) * SafeCastToU32(rg->rectEntityCount);
    rectMemoryTemp.base = (u8 *)AllocateMemory(perFrameMemory, rectMemoryBlockSize);
    rectMemoryTemp.maxSize = rectMemoryBlockSize;

    SortRGRI(rg, &rectMemoryTemp, 0, rg->rectEntityCount);
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

}

void SortDrawEntities()
{

}
