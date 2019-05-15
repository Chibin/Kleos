#ifndef __COMMANDS__
#define __COMMANDS__

void ResetCommandPrompt(GameMetadata *gm)
{
    for(memory_index i = 0; i < ARRAY_SIZE(gm->commandPrompt); i++)
    {
        gm->commandPrompt[i] = '\0';

    }
    gm->commandPromptCount = 0;
}

void WriteToFile(SDL_RWops *file, const char *str)
{
    memory_index len = SDL_strlen(str);
    memory_index bytesWritten = SDL_RWwrite(file, str, 1, len);
    ASSERT(bytesWritten == len);
}

s32 ToChar(char *buffer, memory_index bufSize, v2 a)
{
    return snprintf(buffer, bufSize, "%f, %f", a.x, a.y);
}

s32 ToChar(char *buffer, memory_index bufSize, f32 a)
{
    return snprintf(buffer, bufSize, "%f", a);
}

void SaveScene(GameMetadata *gm, const char *fileName)
{
    SDL_RWops *file = SDL_RWFromFile(fileName, "w+");
    if (file)
    {
        for(memory_index i = 0; i < gm->rectManager->NonTraversable.rda.size; i++)
        {
            Rect *rect = gm->rectManager->NonTraversable.rda.rects[i];

            const char *nameTag = "name:\n";
            WriteToFile(file, nameTag);

            char *name = "saveMap\n";
            WriteToFile(file, name);

            char *textureTag = "texture:\n";
            WriteToFile(file, textureTag);

            char *texture = "box\n";
            WriteToFile(file, texture);

            char *uvCoordsTag = "uv coordinates:\n";
            WriteToFile(file, uvCoordsTag);

            char *uvStart = "[\n";
            WriteToFile(file, uvStart);

            /* TODO: This might turn to some dynamic uv coords later on */
            char *uvCoords = "1, 1,\n1, 0,\n0, 0,\n0, 1,\n";
            WriteToFile(file, uvCoords);

            char *uvEnd = "]\n";
            WriteToFile(file, uvEnd);

            char *widthTag = "width:\n";
            WriteToFile(file, widthTag);

            char dimBuffer[256] = {};
            s32 bytesWritten = ToChar(dimBuffer, sizeof(dimBuffer), rect->width);
            dimBuffer[bytesWritten] = '\n';
            WriteToFile(file, dimBuffer);

            char *heightTag = "height:\n";
            WriteToFile(file, heightTag);

            memset(dimBuffer, 0, sizeof(dimBuffer));
            bytesWritten = ToChar(dimBuffer, sizeof(dimBuffer), rect->height);
            dimBuffer[bytesWritten] = '\n';
            WriteToFile(file, dimBuffer);

            char *objectCountTag = "object count:\n";
            WriteToFile(file, objectCountTag);

            char *object = "1\n";
            WriteToFile(file, object);

            char *pointsTag = "points:\n";
            WriteToFile(file, pointsTag);

            char *pointsStart = "[\n";
            WriteToFile(file, pointsStart);

            /* TODO: Write points here */
            char pointBuf[256] = {};
            bytesWritten = ToChar(pointBuf, sizeof(pointBuf), V2(rect->basePosition));
            WriteToFile(file, pointBuf);
            WriteToFile(file, ",\n");

            char *pointsEnd = "]\n";
            WriteToFile(file, pointsEnd);

        }
    }
    SDL_RWclose(file);
}

void ProcessCommand(GameMetadata *gm, Camera *camera)
{
    if (gm->commandPromptCount == 0)
    {
        return;
    }

    char *command = nullptr;
    char *option = nullptr;

    StringCopy(gm->commandPrompt, gm->backupCommandPrompt, StringLen(gm->commandPrompt));

    char *nextToken = nullptr;
    char *token = strtok_s(gm->commandPrompt, " ", &nextToken);
    ASSERT(token != nullptr);

    if (strcmp(token, "SAVE") == 0 && strcmp(nextToken, "") == 0)
    {
        SaveScene(gm, "save.txt");
    }
    else if (strcmp(token, "SAVE") == 0)
    {
        char buffer[256] = {};
        StringCopy(nextToken, buffer, StringLen(nextToken));
        SaveScene(gm, buffer);
    }
    else if (strcmp(token, "LOAD") == 0)
    {
        ASSERT(!"Load stump");
    }
    else if (strcmp(token, "ZOOM") == 0)
    {
        ASSERT(nextToken != nullptr);

        /* TODO: create a safe cast */
        CameraZoom(camera, (f32)strtol(nextToken, NULL, 10));
    }

    ResetCommandPrompt(gm);
}

#endif
