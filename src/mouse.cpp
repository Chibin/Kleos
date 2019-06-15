#ifndef __MOUSE__
#define __MOUSE__

#include "mouse.h"

b32 IsWithinThreshold(v2 a, v2 b, f32 thresholdValueInScreenCoordinates)
{
    v2 result = abs(a - b);

    return result.x < thresholdValueInScreenCoordinates && result.y < thresholdValueInScreenCoordinates;
}

void UpdateMouseMotion(GameMetadata *gm, Camera *camera, glm::mat4 *projection, SDL_Event &event)
{
    if (gm->mouseInfo.mouseType == LEFT_MOUSE_DRAG)
    {
        gm->mouseInfo.leftScreenCoordinates[1] = V2(GetScreenCoordinateFromMouse(event.motion));
    }
    else if (gm->mouseInfo.mouseType == RIGHT_MOUSE_DRAG)
    {
        gm->mouseInfo.rightScreenCoordinates[1] = V2(GetScreenCoordinateFromMouse(event.motion));
    }
}

void HandleMouseInput(SDL_Event &event, GameMetadata *gm)
{
    f32 screenCoordinatesThresholdValue = 13.0f;

    switch (event.type)
    {
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
            SDL_MouseButtonEvent mbe = event.button;
            switch (mbe.button)
            {
                case SDL_BUTTON_LEFT:
                    if (mbe.state == SDL_PRESSED)
                    {
                        gm->mouseInfo.mouseType = LEFT_MOUSE_DRAG;
                        gm->mouseInfo.leftScreenCoordinates[0] =
                            V2(GetScreenCoordinateFromMouse(event.motion));
                        gm->mouseInfo.leftScreenCoordinates[1] =
                            V2(GetScreenCoordinateFromMouse(event.motion));
                    }
                    else if (mbe.state == SDL_RELEASED)
                    {
                        gm->mouseInfo.leftScreenCoordinates[1] =
                            V2(GetScreenCoordinateFromMouse(event.motion));

                        if (IsWithinThreshold(
                                    gm->mouseInfo.leftScreenCoordinates[0],
                                    gm->mouseInfo.leftScreenCoordinates[1],
                                    screenCoordinatesThresholdValue))
                        {
                            gm->mouseInfo.mouseType = LEFT_SINGLE_CLICK;
                        }
                        else
                        {
                            gm->mouseInfo.mouseType = LEFT_DRAG_CLICK;
                        }

                        gm->mouseInfo.isNew = true;
                    }
                    break;
                case SDL_BUTTON_RIGHT:
                    if (mbe.state == SDL_PRESSED)
                    {
                        gm->mouseInfo.mouseType = RIGHT_MOUSE_DRAG;
                        gm->mouseInfo.rightScreenCoordinates[0] =
                            V2(GetScreenCoordinateFromMouse(event.motion));
                        gm->mouseInfo.rightScreenCoordinates[1] =
                            V2(GetScreenCoordinateFromMouse(event.motion));
                    }
                    else if (mbe.state == SDL_RELEASED)
                    {
                        if (IsWithinThreshold(
                                    gm->mouseInfo.rightScreenCoordinates[0],
                                    gm->mouseInfo.rightScreenCoordinates[1],
                                    screenCoordinatesThresholdValue))
                        {
                            gm->mouseInfo.mouseType = RIGHT_SINGLE_CLICK;
                        }
                        else
                        {
                            gm->mouseInfo.mouseType = RIGHT_DRAG_CLICK;
                        }

                        gm->mouseInfo.isNew = true;
                    }
                    break;
            }
            break;
        case SDL_MOUSEWHEEL:
            /* event.motion doesn't work when the mouse wheel type is
             * happening. I'm not 100% sure why, but getmousestate at
             * least guarantees the mouse position for the focused window */
            s32 x, y;
            SDL_GetMouseState(&x, &y);
            gm->mouseInfo.middleScreenCoordinate = v2{(f32)x, (f32)y};

            if (event.wheel.y == 1)
            {
                gm->mouseInfo.mouseType = MOUSE_WHEEL_UP;
            }
            else if (event.wheel.y == -1)
            {
                gm->mouseInfo.mouseType = MOUSE_WHEEL_DOWN;
            }
            gm->mouseInfo.isNew = true;
            break;
        case SDL_MOUSEMOTION:
            g_mousePoint = ProcessMouseMotion(event.motion);
            UpdateMouseMotion(gm, gm->camera, gm->projection, event);
            break;
        default:
            break;
    }
}
#endif
