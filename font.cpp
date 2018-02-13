#ifndef __FONT__
#define __FONT__

TTF_Font* OpenFont();
inline void CloseFont(TTF_Font *font);

TTF_Font* OpenFont()
{
    const char *file = "assets/fonts/lato/Lato-Regular.ttf";
    int ptsize = 16;
    
    TTF_Font* font = TTF_OpenFont(file, ptsize);
    if (!font) {
        TTF_SetError("Loading failed :( (code: %d)", 142);
        std::cout << "Error: " << TTF_GetError() << std::endl;

        return font;
    }

    printf("font loaded properly.\n");

    return font;
}


SDL_Texture* RenderText(const char* msg,SDL_Renderer *renderer)
{
    TTF_Font *font = OpenFont();
    assert(font != NULL);


    SDL_Color color = {1,1,1,1};

    //render to a surfaceace
    SDL_Surface *surface = TTF_RenderText_Blended(font, msg, color);
    if (surface == NULL){
        CloseFont(font);
        printf("TTF_RenderText\n");
        return NULL;
    }

    //then load that surfaceace into a texture
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL)
        printf("failed to create texture\n");

    //Clean up the surfaceace and font
    SDL_FreeSurface(surface);
    TTF_CloseFont(font);
    return texture;
}

inline void CloseFont(TTF_Font *font)
{
  TTF_CloseFont(font);
}

#endif
