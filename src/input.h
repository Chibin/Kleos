#pragma once

#include "sdl_common.h"

#include "camera.h"
#include "entity.h"

#include "game_metadata.h"

#include "string.h"

void ProcessInputToMovement(SDL_Keycode sym);
void ProcessMouseInput(const SDL_Event &event, Camera *camera);
