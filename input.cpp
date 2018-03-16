#ifndef __INPUT__
#define __INPUT__

/* Should already be included */
#include "camera.cpp"

static bool g_canJump = true;

void ProcessInputToMovement(SDL_Keycode sym);

void ProcessInputDown(SDL_Keycode sym, bool *continueRunning){
    switch(sym){
        case SDLK_ESCAPE:
            *continueRunning = false;
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

void ProcessInputUp(SDL_Keycode sym){
    switch(sym){
        case SDLK_SPACE:
        case SDLK_UP:
            g_canJump = true;
            break;
        default:
            break;
    }
}

void ProcessMouseInput(SDL_Event &event, Camera *camera)
{
     if(event.wheel.y == 1) /* scroll up */
     {
        CameraZoomIn(camera);
     }
     else if(event.wheel.y == -1) /* scroll down */
     {
        CameraZoomOut(camera);
     }

}

void ProcessKeysHeldDown(Entity *entity, const Uint8 *keystate)
{
    if( (keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_SPACE]) && g_canJump){
        EntityMoveUp(entity);
        g_canJump = false;
    }

    if(keystate[SDL_SCANCODE_DOWN]){
        EntityMoveDown(entity);
    }
    if(keystate[SDL_SCANCODE_LEFT]){
        EntityMoveLeft(entity);
    }
    if(keystate[SDL_SCANCODE_RIGHT]){
        EntityMoveRight(entity);
    }

}

void ProcessInputToMovement(SDL_Keycode sym) {
    /* TODO: Replace this with non-repeatable key stroke.
     * Keep repeating the movement while the key is down though.
     */
#if 1
    switch(sym){
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
