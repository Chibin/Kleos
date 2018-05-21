#pragma once

#include "sdl_common.h"

void ProcessInputToMovement(SDL_Keycode sym);
void ProcessMouseInput(const SDL_Event &event, Camera *camera);
