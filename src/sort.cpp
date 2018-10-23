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
        Rect *leftRect = (Rect *)rectMemoryTemp->base+ iLeft;
        Rect *rightRect = (Rect *)rectMemoryTemp->base+ iRight;
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

#if 0
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

void SortDrawEntities()
{

}
