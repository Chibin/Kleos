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

void SaveScene(GameMetadata *gm)
{
    SDL_RWops *file = SDL_RWFromFile("save.txt", "w+");
    if (file)
    {
        const char *str = "Writing to a file\n";
        memory_index len = SDL_strlen(str);
        memory_index bytesWritten = SDL_RWwrite(file, str, 1, len);
        ASSERT( bytesWritten == len);
        printf("Wrote %zu 1-byte blocks\n", len);
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

    if (strcmp(gm->commandPrompt, "SAVE") == 0)
    {
        SaveScene(gm);
    }
    else if (strcmp(token, "SAVE") == 0)
    {
        ASSERT(!"SAVING with a location");
    }
    else if (strcmp(token, "LOAD") == 0)
    {
        ASSERT(!"Load stump");
    }
    else if (strcmp(token, "ZOOM") == 0)
    {
        if (nextToken)
        {
            /* TODO: create a safe cast */
            CameraZoom(camera, (f32)strtol(nextToken, NULL, 10));
        }
    }

    ResetCommandPrompt(gm);
}

#endif
