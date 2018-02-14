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


SDL_Surface *RenderText(TTF_Font *font, const char* msg)
{
    SDL_Color color = {1,1,1,1};

    // Render the message
    SDL_Surface *surface = TTF_RenderText_Blended(font, msg, color);
    if (surface == NULL){
        CloseFont(font);
        printf("TTF_RenderText\n");
        return NULL;
    }
    return surface;
}

inline void CloseFont(TTF_Font *font)
{
  TTF_CloseFont(font);
}

#endif
