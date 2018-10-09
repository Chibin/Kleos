#define MAX_NUM_CHARS 5 /* u8 -> 3 characters, +1 for colon, +1 for \n */

struct PixelCoords{
    v2i pixel[4];
};

struct FrameData
{
    char name[256];
    PixelCoords *frames;
    u8 frameCount;
};

enum FrameDataReadState
{
    NAME,
    FRAME_COUNT,
    FRAMES,
};

char *ReadFrameFile(const char *file, memory_index *o_fileSize)
{
    SDL_RWops *rw = SDL_RWFromFile(file,"r");
    u64 fileSize = 0;
    char *buf = NULL;
    if (rw != NULL) {
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
    // test
    while(string != NULL && *string != '\0')
    {
        if (p == NULL || *string != *p)
        {
            return false;
        }

        p++;
        string++;
    }

    return true;
}

memory_index CopyLine(const char *from, char *to, memory_index bufferSize)
{
    memory_index counter = 0;
    while(counter < bufferSize && from != NULL && *from != '\n')
    {
        if (*from == '\r')
        {
            from++;
            ASSERT(from != NULL && *from == '\n');
            continue;
        }
        to[counter++] = *from;
        from++;
    }
    return counter + strlen("\r\n");
}

memory_index CopyLine(const char *from, u8 *to)
{
    char tmpBuffer[MAX_NUM_CHARS];
    memory_index bytesCopied = CopyLine(from, tmpBuffer, sizeof(tmpBuffer));
    *to = SDL_atoi(tmpBuffer);
    return bytesCopied;
}

memory_index CopyFramePixelCoordinatesFromLine(char *from, v2i *to)
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

    bytesRead += CopyLine(from + bytesRead, tmpBuffer, sizeof(tmpBuffer));
    to->y = SDL_atoi(tmpBuffer);

    return bytesRead;
}

void LoadFrameData(FrameData *fd, const char* file)
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

    while(counter < fileSize)
    {
        switch(fdrs)
        {
            case NAME:
            {

                char *name = "name:\r\n";
                ASSERT( VerifyStringAndGotoNextLine(tmp, name) );
                counter += strlen(name);
                tmp = fileData + counter;

                memory_index bytesCopied = CopyLine(tmp, fd->name, sizeof(fd->name));
                counter = counter + bytesCopied;
                tmp = fileData + counter;

                fdrs = FRAME_COUNT;

                printf("name :%s\n", fd->name);
                break;

            }
            case FRAME_COUNT:
            {

                char *frameCount = "frame count:\r\n";

                ASSERT( VerifyStringAndGotoNextLine(tmp, frameCount) );
                counter += strlen(frameCount);
                tmp = fileData + counter;

                memory_index bytesCopied = CopyLine(tmp, &fd->frameCount);
                counter = counter + bytesCopied;
                tmp = fileData + counter;

                fd->frames = (PixelCoords *)malloc(fd->frameCount * sizeof(PixelCoords));
                memset(fd->frames, 0, fd->frameCount * sizeof(PixelCoords));

                fdrs = FRAMES;

                printf("FRAME COUNT: %d\n", fd->frameCount);
                break;

            }
            case FRAMES:
            {
                for(u8 i = 0; i < fd->frameCount; i++)
                {
                    char frameCounter[5] = {}; /* 3 digits for 2^8, 1 for color, 1 for \n */
                    SDL_uitoa(i, frameCounter, 10);

                    memory_index len = strlen(frameCounter);
                    ASSERT(len != 0);
                    frameCounter[len + 0] = ':';
                    frameCounter[len + 1] = '\r';
                    frameCounter[len + 2] = '\n';

                    printf("frame counter loop: %s\n", frameCounter);
                    
                    counter = counter + strlen(frameCounter);
                    tmp = fileData + counter;

                    for(u8 rectCorner = 0; rectCorner < NUM_OF_RECT_CORNER; rectCorner++)
                    {
                        counter += CopyFramePixelCoordinatesFromLine(tmp, &fd->frames[i].pixel[rectCorner]);
                        printf("x: %d, y: %d\n", fd->frames[i].pixel[rectCorner].x, fd->frames[i].pixel[rectCorner].y);
                        tmp = fileData + counter;
                    }
                }

                fdrs = NAME;
                break;
            }
            default:
                ASSERT(!"SOMETHING BAD HAPPNED");
                break;

        }

        printf("%d\n", fdrs);
    }

    ASSERT(!"SOMETHING");
}
