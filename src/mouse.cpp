enum MouseType
{
    LEFT_SINGLE_CLICK = 0,
    LEFT_DOUBLE_CLICK,
    LEFT_MOUSE_DRAG,
    RIGHT_SINGLE_CLICK,
    RIGHT_DOUBLE_CLICK,
    RIGHT_MOUSE_DRAG,
};

struct MouseInfo
{
    MouseType mouseType;
    /* we need 2 v2 for drag information */
    v2 leftScreenCoordinates[2];
    v2 rightScreenCoordinates[2];
};

void HandleMouseInput(SDL_Event &event, GameMetadata *gameMetadata)
{
    switch (event.type)
    {
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            ProcessMouseEditMode(gameMetadata, gameMetadata->camera, gameMetadata->projection, event);
        case SDL_MOUSEWHEEL:
            ProcessMouseInput(event, gameMetadata->camera);
            break;
        case SDL_MOUSEMOTION:
            g_mousePoint = ProcessMouseMotion(event.motion);
            UpdateMouseDrag(gameMetadata, gameMetadata->camera, gameMetadata->projection, event);
            break;
        default:
            break;
    }

}
