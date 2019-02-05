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

void Transform(Rect *r, glm::mat4 xform)
{
}

Rect *CreateFrameRect(GameMemory *perFrameMemory, FrameState *fs, v3 position, v4 rectColor)
{
    v3 startingPosition = position;

    glm::vec4 xformPosition =
        glm::vec4{fs->currentFrame->pos.x, fs->currentFrame->pos.y, fs->currentFrame->pos.z, 1.0f};

    glm::vec4 xy = glm::vec4{fs->currentFrame->dim.x, fs->currentFrame->dim.y, 0.0f, 1.0f};

    f32 direction = fs->transform[0][0] / abs(fs->transform[0][0]);

    /* We know that the frame will be backwards if the scale is negative */
    xformPosition =  direction * xformPosition;
    xy = fs->transform * xy;

    /* Don't care about the negative value; it will just
     * flip the image upside down.
     */
    f32 rectWidth = abs(xy.x);
    f32 rectHeight = abs(xy.y);

    startingPosition += v3{xformPosition.x, xformPosition.y, xformPosition.z};

    return CreateRectangle(perFrameMemory, startingPosition,
                rectColor, rectWidth, rectHeight);

}
