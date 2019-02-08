#include "asset.h"

char *ReadFrameFile(const char *file, memory_index *o_fileSize)
{
    SDL_RWops *rw = SDL_RWFromFile(file,"r");
    u64 fileSize = 0;
    char *buf = nullptr;
    if (rw != nullptr) {
        fileSize = SDL_RWsize(rw);
        buf = (char *)malloc(fileSize + 1);
        SDL_RWread(rw, buf, fileSize, 1);
        buf[fileSize] = '\0';

        printf("buf: %s\n", buf);

        SDL_RWclose(rw);
    }
    *o_fileSize = fileSize;
    return buf;
}

b32 VerifyStringAndGotoNextLine(char *p, const char *string)
{
    while(string != nullptr && *string != '\0')
    {
        if (p == nullptr || *string != *p)
        {
            return false;
        }

        p++;
        string++;
    }

    return true;
}

memory_index CopyLine(const char *from, char *to, memory_index bufferSize, b32 isNewlineOnly)
{
    memory_index counter = 0;
    while(counter < bufferSize && from != nullptr && *from != '\n')
    {
        if (*from == '\r')
        {
            from++;
            ASSERT(from != nullptr && *from == '\n');
            continue;
        }
        to[counter++] = *from;
        from++;
    }
    if (isNewlineOnly)
    {
        return counter + strlen("\n");
    }

    return counter + strlen("\r\n");
}

memory_index CopyLine(const char *from, u8 *to, b32 isNewlineOnly)
{
    char tmpBuffer[MAX_NUM_CHARS];
    memory_index bytesCopied = CopyLine(from, tmpBuffer, sizeof(tmpBuffer), isNewlineOnly);
    *to = SDL_atoi(tmpBuffer);
    return bytesCopied;
}

memory_index CopyFramePixelCoordinatesFromLine(char *from, v2i *to, b32 isNewlineOnly)
{
    memory_index bytesRead = 0;
    char tmpBuffer[MAX_NUM_CHARS];

    char *pch = strchr(from, ',');
    memcpy((void *)tmpBuffer, (const void *)from, (memory_index)(pch - from));
    to->x = SDL_atoi(tmpBuffer);

    bytesRead = (memory_index)(pch - from);
    ASSERT(*(from + bytesRead + 1) == ' ');
    bytesRead += 2; /* skip the space character */

    memset(tmpBuffer, 0, sizeof(tmpBuffer));

    bytesRead += CopyLine(from + bytesRead, tmpBuffer, sizeof(tmpBuffer), isNewlineOnly);
    to->y = SDL_atoi(tmpBuffer);

    return bytesRead;
}

void LoadFrameData(FrameAnimation *fa, const char* file)
{
    memory_index fileSize = 0;
    char *fileData = ReadFrameFile(file, &fileSize);
    memory_index counter = 0;
    FrameDataReadState fdrs = NAME;
    u8 frameCount = 0;

    /* All 3 of them represent the end of a line. But...
     * \r (Carriage Return) - moves the cursor to the beginning of the line without advancing to the next line
     * \n (Line Feed) - moves the cursor down to the next line without returning to the beginning of the line - In a *nix environment \n moves to the beginning of the line.
     * \r\n (End Of Line) - a combination of \r and \n
     * https://stackoverflow.com/questions/15433188/r-n-r-n-what-is-the-difference-between-them
     */
    char *tmp = fileData;
    b32 isNewlineOnly = false;
    u8 animationStateCount = 0;

    while(counter < fileSize)
    {
        switch(fdrs)
        {
            case NAME:
            {

                char *name = "name:\r\n";
                if (VerifyStringAndGotoNextLine(tmp, name) == false)
                {
                    name = "name:\n";
                    ASSERT(VerifyStringAndGotoNextLine(tmp, name));
                    isNewlineOnly = true;
                }
                counter += strlen(name);
                tmp = fileData + counter;

                memory_index bytesCopied =
                    CopyLine(tmp, fa->animationName, sizeof(fa->animationName), isNewlineOnly);
                counter = counter + bytesCopied;
                tmp = fileData + counter;

                fdrs = ANIMATION_COUNT;

                printf("name :%s\n", fa->animationName);
                break;

            }
            case ANIMATION_COUNT:
            {
                char *animationCount = "animation count:\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, animationCount));

                counter += strlen(animationCount);
                tmp = fileData + counter;

                memory_index bytesCopied = CopyLine(tmp, &fa->animationCount, true);
                counter = counter + bytesCopied;
                tmp = fileData + counter;

                fa->frameCycles = (FrameCycle *)malloc(fa->animationCount * sizeof(FrameCycle));
                memset(fa->frameCycles, 0, fa->animationCount * sizeof(FrameCycle));

                fdrs = FRAME_STATE_NAME;
                printf("animation count: %d\n", fa->animationCount);
                break;
            }
            case FRAME_STATE_NAME:
            {
                ASSERT(animationStateCount <= fa->animationCount);
                FrameCycle *fc = &fa->frameCycles[animationStateCount];

                char *name = "frame name:\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, name));

                counter += strlen(name);
                tmp = fileData + counter;

                memory_index bytesCopied =
                    CopyLine(tmp, fc->name, sizeof(fc->name), true);
                counter = counter + bytesCopied;
                tmp = fileData + counter;

                fdrs = FRAME_COUNT;
                printf("frame name: %s\n", fc->name);

                break;
            }
            case FRAME_COUNT:
            {
                FrameCycle *fc = &fa->frameCycles[animationStateCount];

                char *frameCount = "frame count:\r\n";
                if (VerifyStringAndGotoNextLine(tmp, frameCount) == false)
                {
                    frameCount = "frame count:\n";
                    ASSERT(VerifyStringAndGotoNextLine(tmp, frameCount));
                    isNewlineOnly = true;
                }

                counter += strlen(frameCount);
                tmp = fileData + counter;

                memory_index bytesCopied = CopyLine(tmp, &fc->frameCount, isNewlineOnly);
                counter = counter + bytesCopied;
                tmp = fileData + counter;

                fc->frames = (PixelCoords *)malloc(fc->frameCount * sizeof(PixelCoords));
                memset(fc->frames, 0, fc->frameCount * sizeof(PixelCoords));

                fdrs = FRAMES;

                printf("FRAME COUNT: %d\n", fc->frameCount);
                break;

            }
            case FRAMES:
            {
                FrameCycle *fc = &fa->frameCycles[animationStateCount];

                for(u8 i = 0; i < fc->frameCount; i++)
                {
                    char frameCounter[5] = {}; /* 3 digits for 2^8, 1 for color, 1 for \n */
                    SDL_uitoa(i, frameCounter, 10);

                    memory_index len = strlen(frameCounter);
                    ASSERT(len != 0);
                    frameCounter[len + 0] = ':';
                    if (isNewlineOnly)
                    {
                        frameCounter[len + 1] = '\n';
                    }
                    else
                    {
                        frameCounter[len + 1] = '\r';
                        frameCounter[len + 2] = '\n';
                    }

                    printf("frame counter loop: %s\n", frameCounter);

                    counter = counter + strlen(frameCounter);
                    tmp = fileData + counter;

                    for(u8 rectCorner = 0; rectCorner < NUM_OF_RECT_CORNER; rectCorner++)
                    {
                        counter += CopyFramePixelCoordinatesFromLine(tmp, &fc->frames[i].pixel[rectCorner], isNewlineOnly);
                        printf("x: %d, y: %d\n", fc->frames[i].pixel[rectCorner].x, fc->frames[i].pixel[rectCorner].y);
                        tmp = fileData + counter;
                    }
                }

                if (animationStateCount < fa->animationCount)
                {
                    fdrs = FRAME_STATE_NAME;
                    animationStateCount++;
                }
                else
                {
                    fdrs = NAME;
                    animationStateCount = 0;
                }
                break;
            }
            default:
                ASSERT(!"SOMETHING BAD HAPPNED");
                break;

        }

        printf("framedata read state: %d\n", fdrs);
    }

}

/*
 * ANIMATION
 * TODO: This needs to change to a hash map if it gets too big
 */

void AddFrameAnimationNode(FrameAnimation *oldNode, FrameAnimation *newNode)
{
    newNode->next = oldNode->next;
    newNode->prev = oldNode;
    oldNode->next->prev = newNode;
    oldNode->next = newNode;
}

FrameAnimation *GetFrameAnimation(FrameAnimation *sentinelNode, const char* name)
{
    for (FrameAnimation *node = sentinelNode->next; node != sentinelNode; node = node->next)
    {
        if (strcmp(node->animationName, name) == 0)
        {
            return node;
        }
    }

    return nullptr;
}

Animation2D *GetSpriteAnimationInfo(FrameAnimation *fa, const char *name)
{
    for (memory_index i = 0; i < fa->animationCount; i++)
    {
        if (strcmp(fa->frameCycles[i].name, name) == 0)
        {
            return fa->frameCycles[i].animationInfo;
        }
    }

    ASSERT(!"I shouldn't get here");

    return nullptr;
}
