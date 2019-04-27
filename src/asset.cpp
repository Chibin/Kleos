#include "asset.h"

char *ReadFile(const char *file, memory_index *o_fileSize)
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
    *to = SafeCastToU8(SDL_atoi(tmpBuffer));
    return bytesCopied;
}

memory_index CopyLine(const char *from, u32 *to, b32 isNewlineOnly)
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

void LoadFrameData(GameMetadata *gameMetadata, const char* file)
{
    memory_index fileSize = 0;
    char *fileData = ReadFile(file, &fileSize);
    memory_index counter = 0;
    FrameDataReadState fdrs = NAME;
    GameMemory *reservedMemory = &gameMetadata->reservedMemory;

    /* All 3 of them represent the end of a line. But...
     * \r (Carriage Return) - moves the cursor to the beginning of the line without advancing to the next line
     * \n (Line Feed) - moves the cursor down to the next line without returning to the beginning of the line - In a *nix environment \n moves to the beginning of the line.
     * \r\n (End Of Line) - a combination of \r and \n
     * https://stackoverflow.com/questions/15433188/r-n-r-n-what-is-the-difference-between-them
     */
    char *tmp = fileData;
    b32 isNewlineOnly = false;
    u8 animationStateCount = 0;

    FrameAnimation *fa = nullptr;
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

                fa = (FrameAnimation *)AllocateMemory(reservedMemory, sizeof(FrameAnimation));
                ZeroSize(fa, sizeof(FrameAnimation));

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
                ASSERT(animationStateCount < fa->animationCount);
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

                char *name = "frame count:\r\n";
                if (VerifyStringAndGotoNextLine(tmp, name) == false)
                {
                    name = "frame count:\n";
                    ASSERT(VerifyStringAndGotoNextLine(tmp, name));
                    isNewlineOnly = true;
                }

                counter += strlen(name);
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

                if (animationStateCount < fa->animationCount - 1)
                {
                    fdrs = FRAME_STATE_NAME;
                    animationStateCount++;
                }
                else
                {
                    fdrs = NAME;
                    animationStateCount = 0;
                    AddFrameAnimationNode(&gameMetadata->frameAnimationSentinelNode, fa);
                }
                break;
            }
            default:
                ASSERT(!"SOMETHING BAD HAPPNED");
                break;

        }

        printf("framedata read state: %d\n", fdrs);
    }

    free(fileData);

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

struct MapData
{
    char name[16];
    char textureName[16];
    RectUVCoords uvCoords;
    v2 dim;
    u32 count;
    v2 *basePoints;

    MapData *next;

    MinMax aabbMinMax;
};

enum MapDataReadState
{
    MAP_NAME,
    MAP_TEXTURE_NAME,
    MAP_UV_COORDINATES,
    MAP_RECT_HEIGHT,
    MAP_RECT_WIDTH,
    MAP_OBJECT_COUNT,
    MAP_BASE_POINTS,
};

/* MAP asset reader */
MapData *LoadAssetMap(const char *file)
{
    memory_index fileSize = 0;
    char *fileData = ReadFile(file, &fileSize);
    ASSERT(fileData != nullptr);

    memory_index counter = 0;
    MapDataReadState fdrs = MAP_NAME;

    char *tmp = fileData;
    b32 isNewlineOnly = true;

    MapData *rootMDNode = (MapData *)malloc(sizeof(MapData));
    ZeroSize(rootMDNode, sizeof(MapData));

    MapData *currentNode = nullptr;
    currentNode = rootMDNode;

    while(counter < fileSize)
    {
        switch(fdrs)
        {
            case MAP_NAME:
            {

                char *name = "name:\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, name) == true);

                MapData *newNode = (MapData *)malloc(sizeof(MapData));
                ZeroSize(newNode, sizeof(MapData));

                currentNode->next = newNode;
                currentNode = newNode;

                counter += strlen(name);
                tmp = fileData + counter;

                memory_index bytesCopied =
                    CopyLine(tmp, currentNode->name, sizeof(currentNode->name), isNewlineOnly);
                counter = counter + bytesCopied;
                tmp = fileData + counter;

                fdrs = MAP_TEXTURE_NAME;

                printf("name: %s\n", currentNode->name);
                break;
            }
            case MAP_TEXTURE_NAME:
            {
                char *name = "texture:\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, name) == true);

                counter += strlen(name);
                tmp = fileData + counter;

                memory_index bytesCopied =
                    CopyLine(tmp, currentNode->textureName, sizeof(currentNode->textureName), isNewlineOnly);
                counter = counter + bytesCopied;
                tmp = fileData + counter;

                fdrs = MAP_UV_COORDINATES;

                printf("texture name: %s\n", currentNode->textureName);
                break;
            }
            case MAP_UV_COORDINATES:
            {
                char *name = "uv coordinates:\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, name) == true);
                counter += strlen(name);
                tmp = fileData + counter;

                char *openBracket = "[\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, openBracket) == true);
                counter += strlen(openBracket);

                for(memory_index i = 0; i < 8; i += 2)
                {
                    tmp = fileData + counter;

                    char tmpBuffer[8];
                    char *pch = strchr(tmp, ',');
                    memory_index bytesRead = 0;
                    memcpy((void *)tmpBuffer, (const void *)tmp, (memory_index)(pch - tmp));
                    currentNode->uvCoords.UV[SafeCastToU32(i/2)].x = f32(atof(tmpBuffer));

                    bytesRead = (memory_index)(pch - tmp);
                    ASSERT(*(tmp + bytesRead + 1) == ' ');
                    bytesRead += 2; /* skip the space character */

                    counter += bytesRead;

                    tmp = fileData + counter;
                    memset(tmpBuffer, 0, sizeof(tmpBuffer));
                    bytesRead = 0;
                    pch = strchr(tmp, ',');
                    memcpy((void *)tmpBuffer, (const void *)tmp, (memory_index)(pch - tmp));
                    currentNode->uvCoords.UV[SafeCastToU32(i/2)].y = f32(atof(tmpBuffer));
                    bytesRead += (memory_index)(pch - tmp);
                    bytesRead += strlen(",\n"); /* ,\n skip these characters */

                    counter += bytesRead;
                }

                tmp = fileData + counter;

                char *closeBracket = "]\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, closeBracket) == true);
                counter += strlen(closeBracket);
                tmp = fileData + counter;

                fdrs = MAP_RECT_WIDTH;
                break;
            }
            case MAP_RECT_WIDTH:
            {
                char *name = "width:\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, name) == true);

                counter += strlen(name);
                tmp = fileData + counter;

                char tmpBuffer[256];
                memory_index bytesCopied =
                    CopyLine(tmp, tmpBuffer, sizeof(tmpBuffer), isNewlineOnly);
                counter = counter + bytesCopied;
                tmp = fileData + counter;

                currentNode->dim.x = (f32)atof(tmpBuffer);
                ASSERT(currentNode->dim.x != 0.0);

                fdrs = MAP_RECT_HEIGHT;
                break;
            }
            case MAP_RECT_HEIGHT:
            {
                char *name = "height:\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, name) == true);

                counter += strlen(name);
                tmp = fileData + counter;

                char tmpBuffer[256];
                memory_index bytesCopied =
                    CopyLine(tmp, tmpBuffer, sizeof(tmpBuffer), isNewlineOnly);
                counter = counter + bytesCopied;
                tmp = fileData + counter;

                currentNode->dim.y = (f32)atof(tmpBuffer);
                ASSERT(currentNode->dim.y != 0.0);

                fdrs = MAP_OBJECT_COUNT;
                break;
            }
            case MAP_OBJECT_COUNT:
            {
                char *name = "object count:\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, name) == true);

                counter += strlen(name);
                tmp = fileData + counter;

                char tmpBuffer[256];
                memory_index bytesCopied =
                    CopyLine(tmp, tmpBuffer, sizeof(tmpBuffer), isNewlineOnly);
                counter = counter + bytesCopied;
                tmp = fileData + counter;

                currentNode->count = SDL_atoi(tmpBuffer);
                ASSERT(currentNode->count != 0);

                fdrs = MAP_BASE_POINTS;
                break;
            }
            case MAP_BASE_POINTS:
            {
                const char *name = "points:\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, name) == true);

                counter += strlen(name);
                tmp = fileData + counter;

                char *openBracket = "[\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, openBracket) == true);
                counter += strlen(openBracket);

                currentNode->basePoints = (v2 *)malloc(sizeof(v2) * currentNode->count);
                for(memory_index i = 0; i < currentNode->count; i++)
                {
                    tmp = fileData + counter;

                    char tmpBuffer[8];
                    char *pch = strchr(tmp, ',');
                    memory_index bytesRead = 0;
                    memcpy((void *)tmpBuffer, (const void *)tmp, (memory_index)(pch - tmp));
                    currentNode->basePoints[i].x = (f32)atof(tmpBuffer);

                    bytesRead = (memory_index)(pch - tmp);
                    ASSERT(*(tmp + bytesRead + 1) == ' ');
                    bytesRead += 2; /* skip the space character */

                    counter += bytesRead;

                    tmp = fileData + counter;
                    memset(tmpBuffer, 0, sizeof(tmpBuffer));
                    bytesRead = 0;
                    pch = strchr(tmp, ',');
                    memcpy((void *)tmpBuffer, (const void *)tmp, (memory_index)(pch - tmp));
                    currentNode->basePoints[i].y = (f32)atof(tmpBuffer);
                    bytesRead += (memory_index)(pch - tmp);
                    bytesRead += strlen(",\n"); /* ,\n skip these characters */

                    counter += bytesRead;

                }

                tmp = fileData + counter;

                char *closeBracket = "]\n";
                ASSERT(VerifyStringAndGotoNextLine(tmp, closeBracket) == true);
                counter += strlen(closeBracket);
                tmp = fileData + counter;

                fdrs = MAP_NAME;
                break;
            }
            default:
            {
                ASSERT(!"SOMETHING BAD HAPPNED");
                break;
            }
        }
    }

    free(fileData);

    return rootMDNode;
}

void SetAABB(MapData *rootMDNode)
{
    MinMax aabbMinMax = {};
    for(MapData *currentNode = rootMDNode->next; currentNode != nullptr; currentNode = currentNode->next)
    {
        for (memory_index i = 0; i < currentNode->count; i++)
        {

            v2 center = currentNode->basePoints[i];
            v2 dim = currentNode->dim;
            MinMax result = GetMinMax(center, dim);

            if (result.min < aabbMinMax.min)
            {
                aabbMinMax.min = result.min;
            }
            if (aabbMinMax.max < result.max)
            {
                aabbMinMax.max = result.max;
            }
        }
    }

    rootMDNode->aabbMinMax = aabbMinMax;
}
