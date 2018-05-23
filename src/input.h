#pragma once

#include "sdl_common.h"

#include "camera.h"
#include "entity.h"

void ProcessInputToMovement(SDL_Keycode sym);
void ProcessMouseInput(const SDL_Event &event, Camera *camera);
