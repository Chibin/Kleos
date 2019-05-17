#ifndef __INPUT__
#define __INPUT__
#include "input.h"

static bool g_canJump = true;
static bool g_canAttack = true;

void Toggle(b32 *value)
{
    *value = !*value;
}

void ProcessInputDown(
        SDL_Keycode sym,
        GameMetadata *gm,
        Camera *camera,
        b32 *continueRunning)
{
    if (gm->isCommandPrompt && gm->isEditMode)
    {
        switch (sym)
        {
            case SDLK_ESCAPE:
                ResetCommandPrompt(gm);
                gm->isCommandPrompt = false;
                break;
            case SDLK_RETURN:
                if(gm->isEditMode)
                {
                    ProcessCommand(gm, camera);
                    Toggle(&gm->isCommandPrompt);
                }
                break;
            case SDLK_BACKSPACE:
                if(gm->commandPromptCount > 0)
                {
                    gm->commandPrompt[--gm->commandPromptCount] = '\0';
                }
                break;
            case SDLK_UP:
                StringCopy(gm->backupCommandPrompt, gm->commandPrompt, sizeof(gm->backupCommandPrompt));
                gm->commandPromptCount = StringLen(gm->commandPrompt);
                break;
            default:
                ASSERT(gm->commandPromptCount < sizeof(gm->commandPrompt));
                if (sym == SDLK_SPACE)
                {
                    gm->commandPrompt[gm->commandPromptCount++] = ' ';
                }
                else
                {
                    gm->commandPrompt[gm->commandPromptCount++] = *SDL_GetKeyName(sym);
                }
                break;
        }
    }
    else
    {
        switch (sym)
        {
            case SDLK_ESCAPE:
                *continueRunning = false;
                break;
            case SDLK_1:
                break;
            case SDLK_p:
                Toggle(&gm->gameTimestep->isPaused);
                break;
            case SDLK_e:
                Toggle(&gm->isEditMode);
                break;
            case SDLK_RETURN:
                Toggle(&gm->isCommandPrompt);
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
            break;
        case SDL_BUTTON_RIGHT:
            break;
        case SDL_BUTTON_MIDDLE:
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
            break;
        case SDL_BUTTON_RIGHT:
            break;
        case SDL_BUTTON_MIDDLE:
            break;
        default:
            ASSERT(!"I shouldn't get here");
            break;
    }
}

#define SCREEN_WIDTH 1366
#define SCREEN_HEIGHT 768

v3 GetScreenCoordinateFromMouse(const SDL_MouseMotionEvent &mme)
{
    return v3{(f32)mme.x, (f32)mme.y, 0};
}

v3 ProcessMouseMotion(const SDL_MouseMotionEvent &mme)
{
    return GetScreenCoordinateFromMouse(mme);
}

void ProcessMouseInput(const SDL_Event &event, Camera *camera)
{
    if (event.wheel.y == 1) /* scroll up */
    {
        CameraZoom(camera, -1.0f);
    }
    else if (event.wheel.y == -1) /* scroll down */
    {
        CameraZoom(camera, 1.0f);
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
