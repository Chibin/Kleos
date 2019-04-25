#ifndef __INPUT__
#define __INPUT__
#include "input.h"

static bool g_canJump = true;
static bool g_canAttack = true;

void ToggleBetweenVulkanAndOpenGL(GameMetadata *gm)
{
    gm->isVulkanActive = !gm->isVulkanActive;
    gm->isOpenGLActive = !gm->isVulkanActive;
}

void TogglePauseGame(GameMetadata *gm)
{
    if (IsGamePaused(gm->gameTimestep))
    {
        UnpauseGameTimeStep(gm->gameTimestep);
    }
    else
    {
        PauseGameTimestep(gm->gameTimestep);
    }
}

void ProcessInputDown(
        SDL_Keycode sym,
        GameMetadata *gm,
        b32 *continueRunning)
{
    switch (sym)
    {
    case SDLK_ESCAPE:
        *continueRunning = false;
        break;
    case  SDLK_1:
        ToggleBetweenVulkanAndOpenGL(gm);
        break;
    case SDLK_p:
        TogglePauseGame(gm);
        break;
    default:
        /* TODO: differentiate different types of input
         * something like...  if (mode == movement)
         */
        ProcessInputToMovement(sym);
        /* TODO: ProcessInputToMenu() */
        break;
    }
}

void ProcessInputUp(SDL_Keycode sym)
{
    switch (sym)
    {
    case SDLK_SPACE:
    case SDLK_UP:
        g_canJump = true;
        break;
    case SDLK_s:
        g_canAttack = true;
        break;
    default:
        break;
    }
}

void ProcessMouseButtonReleased(u8 button)
{
    switch (button)
    {
        case SDL_BUTTON_LEFT:
            ASSERT(!"LEFT RELEASE");
            break;
        case SDL_BUTTON_RIGHT:
            ASSERT(!"right release");
            break;
        case SDL_BUTTON_MIDDLE:
            ASSERT(!"middle release");
            break;
        default:
            ASSERT(!"I shouldn't get here");
            break;
    }
}

void ProcessMouseButtonPressed(u8 button)
{
    switch (button)
    {
        case SDL_BUTTON_LEFT:
            ASSERT(!"LEFT PRESSED\n");
            break;
        case SDL_BUTTON_RIGHT:
            ASSERT(!"right pressed");
            break;
        case SDL_BUTTON_MIDDLE:
            ASSERT(!"middle pressed");
            break;
        default:
            ASSERT(!"I shouldn't get here");
            break;
    }
}

void ProcessMouseButton(const SDL_MouseButtonEvent &mbe)
{
    if (mbe.state == SDL_RELEASED)
    {
        ProcessMouseButtonReleased(mbe.button);
    }
    else if (mbe.state == SDL_PRESSED)
    {
        ProcessMouseButtonPressed(mbe.button);
    }
}

#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768

v3 ProcessMouseMotion(const SDL_MouseMotionEvent &mme, Camera *camera)
{
    printf("X %d, Y %d\n", mme.x, mme.y);
    v2 normalizedMME = {mme.x/(f32)SCREEN_WIDTH, mme.y/(f32)SCREEN_HEIGHT};

    //v2 worldCoord = GetCenterOfWorldCoordinatesOnScreen();

    /* TODO: Translate to world coordinates.
     * Probably doable with the camera?
     */
    //v2 pos = TranslateToWorldCoordinates(v2{x, y});
#if 0
    v2 screenCoordinates = 2 * normalizedMME - 1.0f;
    screenCoordinates.y = -screenCoordinates.y;
    return v3{screenCoordinates.x, screenCoordinates.y, 0};
#else
    return v3{(f32)mme.x, (f32)mme.y, 0};
#endif

}

void ProcessMouseInput(const SDL_Event &event, Camera *camera)
{
    if (event.wheel.y == 1) /* scroll up */
    {
        CameraZoomIn(camera);
    }
    else if (event.wheel.y == -1) /* scroll down */
    {
        CameraZoomOut(camera);
    }
}

void ProcessKeysHeldDown(Entity *entity, const u8 *keystate)
{
    if (((keystate[SDL_SCANCODE_UP] != 0u) ||
         (keystate[SDL_SCANCODE_SPACE] != 0u)) &&
        g_canJump)
    {
        EntityMoveUp(entity);
        g_canJump = false;
    }
    if ((keystate[SDL_SCANCODE_S] != 0u) && g_canAttack)
    {
        EntityMoveAttack(entity);
        g_canAttack = false;
    }

    if (keystate[SDL_SCANCODE_DOWN] != 0u)
    {
        EntityMoveDown(entity);
    }
    if (keystate[SDL_SCANCODE_LEFT] != 0u)
    {
        EntityMoveLeft(entity);
    }
    if (keystate[SDL_SCANCODE_RIGHT] != 0u)
    {
        EntityMoveRight(entity);
    }
}

void ProcessInputToMovement(SDL_Keycode sym)
{
/* TODO: Replace this with non-repeatable key stroke.
 * Keep repeating the movement while the key is down though.
 */
#if 1
    switch (sym)
    {
    case SDLK_d:
        break;
    case SDLK_UP:
        break;
    case SDLK_DOWN:
        break;
    case SDLK_LEFT:
        break;
    case SDLK_RIGHT:
        break;
    default:
        break;
    }
#endif
}

#endif
