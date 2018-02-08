#ifndef __INPUT__
#define __INPUT__

void HandleInput(SDL_Keycode sym, bool *continueRunning, SDL_Window *mainWindow){
    switch(sym){
            case SDLK_ESCAPE:
                *continueRunning = false;
                break;
            case SDLK_r:
                // Cover with red and update
                glClearColor(1.0, 0.0, 0.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT);
                SDL_GL_SwapWindow(mainWindow);
                break;
            case SDLK_g:
                // Cover with green and update
                glClearColor(0.0, 1.0, 0.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT);
                SDL_GL_SwapWindow(mainWindow);
                break;
            case SDLK_b:
                // Cover with blue and update
                glClearColor(0.0, 0.0, 1.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT);
                SDL_GL_SwapWindow(mainWindow);
                break;
            default:
                break;
    }
}

#endif
