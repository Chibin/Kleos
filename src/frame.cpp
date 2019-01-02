/* Return false when we've hit the last frame
 * Return true if there exists another frame
 * or the frame is still playing
 */
b32 UpdateFrameState(FrameState *fs, u32 dt)
{
    fs->timePassedCurrentFrame += dt;
    if (fs->timePassedCurrentFrame > fs->currentFrame->duration)
    {

        fs->currentFrame = fs->currentFrame->next;
        if (fs->currentFrame == NULL)
        {
            fs->currentFrame = fs->startFrame;
        }
        fs->timePassedCurrentFrame = 0;
        return false;
    }
    return true;
}

Rect *CreateFrameRect(GameMemory *perFrameMemory, FrameState *fs, v3 position, v4 rectColor)
{
    v3 startingPosition = position;
    startingPosition += fs->currentFrame->pos;
    f32 rectWidth = fs->currentFrame->dim.x;
    f32 rectHeight = fs->currentFrame->dim.y;

    return CreateRectangle(perFrameMemory, startingPosition,
                rectColor, rectWidth, rectHeight);
}
