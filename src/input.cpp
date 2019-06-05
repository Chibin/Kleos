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
    if (gm->editMode.isCommandPrompt && gm->editMode.isActive)
    {
        switch (sym)
        {
            case SDLK_ESCAPE:
                ResetCommandPrompt(gm);
                gm->editMode.isCommandPrompt = false;
                break;
            case SDLK_RETURN:
                if(gm->editMode.isActive)
                {
                    ProcessCommand(gm, camera);
                    Toggle(&gm->editMode.isCommandPrompt);
                }
                break;
            case SDLK_BACKSPACE:
                if(gm->editMode.commandPromptCount > 0)
                {
                    gm->editMode.commandPrompt[--gm->editMode.commandPromptCount] = '\0';
                }
                break;
            case SDLK_UP:
                StringCopy(gm->editMode.backupCommandPrompt, gm->editMode.commandPrompt, sizeof(gm->editMode.backupCommandPrompt));
                gm->editMode.commandPromptCount = StringLen(gm->editMode.commandPrompt);
                break;
            default:
                ASSERT(gm->editMode.commandPromptCount < sizeof(gm->editMode.commandPrompt));
                if (sym == SDLK_SPACE)
                {
                    gm->editMode.commandPrompt[gm->editMode.commandPromptCount++] = ' ';
                }
                else
                {
                    gm->editMode.commandPrompt[gm->editMode.commandPromptCount++] = *SDL_GetKeyName(sym);
                }
                break;
        }
    }
    else
    {
        switch (sym)
        {
            case SDLK_ESCAPE:
                if (gm->editMode.isActive)
                {
                    Toggle(&gm->editMode.isActive);
                    ResetCommandPrompt(gm);
                    gm->editMode.isCommandPrompt = false;

                    /* Only need to set this value when you're getting out of edit mode. */
                    gm->editMode.selectedRect = nullptr;
                }
                else
                {
                    *continueRunning = false;
                }
                break;
            case SDLK_1:
                break;
            case SDLK_p:
                Toggle(&gm->gameTimestep->isPaused);
                break;
            case SDLK_e:
                Toggle(&gm->editMode.isActive);
                /* Only need to set this value when you're getting out of edit mode. */
                gm->editMode.selectedRect = nullptr;
                break;
            case SDLK_RETURN:
                /* only activatecommand prompt when edit mode is active */
                if (gm->editMode.isActive)
                {
                    Toggle(&gm->editMode.isCommandPrompt);
                }
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
    //printf("X: %d, Y: %d\n", mme.x, mme.y);
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

void ProcessKeysHeldDown(Hash *hash, Entity *entity, const u8 *keystate)
{
    Movement *movement = HashGetValue(HashEntityMovement, hash, entity);

    if (((keystate[SDL_SCANCODE_UP] != 0u) ||
         (keystate[SDL_SCANCODE_SPACE] != 0u)) &&
        g_canJump)
    {
        MoveUp(movement);
        g_canJump = false;
    }
    if ((keystate[SDL_SCANCODE_S] != 0u) && g_canAttack)
    {
        EntityMoveAttack(entity);
        g_canAttack = false;
    }

    if (keystate[SDL_SCANCODE_DOWN] != 0u)
    {
        MoveDown(movement);
    }
    if (keystate[SDL_SCANCODE_LEFT] != 0u)
    {
        MoveLeft(movement);
    }
    if (keystate[SDL_SCANCODE_RIGHT] != 0u)
    {
        MoveRight(movement);
    }
}

void ProcessKeysHeldDownEditMode(GameMetadata *gm, const u8 *keystate)
{
    Rect *rect = gm->editMode.selectedRect;
    v3 positionOffset = {0,0,0};
    f32 moveBy = 0.05f;

    if (keystate[SDL_SCANCODE_UP] != 0u)
    {
        positionOffset += v3{0.0f, moveBy, 0.0f};
    }
    if (keystate[SDL_SCANCODE_DOWN] != 0u)
    {
        positionOffset += v3{0.0f, -moveBy, 0.0f};
    }
    if (keystate[SDL_SCANCODE_LEFT] != 0u)
    {
        positionOffset += v3{-moveBy, 0.0f, 0.0f};
    }
    if (keystate[SDL_SCANCODE_RIGHT] != 0u)
    {
        positionOffset += v3{moveBy, 0.0f, 0.0f};
    }

    rect->center += positionOffset;
    UpdatePosition(rect, rect->center);
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
