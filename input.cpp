#ifndef __INPUT__
#define __INPUT__

void ProcessInputToMovement(SDL_Keycode sym, entity *Entity);

void ProcessInput(SDL_Keycode sym, bool *continueRunning, entity *player){
    switch(sym){
        case SDLK_ESCAPE:
            *continueRunning = false;
            break;
        default:
            /* TODO: differentiate different types of input
             * something like...  if (mode == movement) 
             */
            ProcessInputToMovement(sym, player);
            /* TODO: ProcessInputToMenu() */
            break;
    }
}

void ProcessInputToMovement(SDL_Keycode sym, entity *Entity) {
    /* TODO: Replace this with non-repeatable key stroke.
     * Keep repeating the movement while the key is down though.
     */
    switch(sym){
        case SDLK_d:
            break;
        case SDLK_UP:
            EntityMoveUp(Entity);
            printf("up\n");
            break;
        case SDLK_DOWN:
            EntityMoveDown(Entity);
            printf("down\n");
            break;
        case SDLK_LEFT:
            EntityMoveLeft(Entity);
            printf("left\n");
            break;
        case SDLK_RIGHT:
            EntityMoveRight(Entity);
            printf("right\n");
            break;
        default:
            break;
    }
}

#endif
