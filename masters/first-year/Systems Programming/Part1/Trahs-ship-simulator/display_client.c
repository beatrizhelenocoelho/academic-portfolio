#include <SDL2/SDL.h>
#include <stdio.h>
#include <SDL2/SDL_ttf.h>


///////////////////////////////////////////////
// To i can draw arrow on screen
///////////////////////////////////////////////
void draw_text(SDL_Renderer* renderer, TTF_Font* font, const char* message)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_Color white = {255, 255, 255, 255};

    SDL_Surface* surface = TTF_RenderUTF8_Solid(font, message, white);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!texture) return;

    int w, h;
    SDL_QueryTexture(texture, NULL, NULL, &w, &h);

    SDL_Rect dst = {150, -20, w, h};

    SDL_RenderCopy(renderer, texture, NULL, &dst);

    SDL_DestroyTexture(texture);
}
/////////////////////////////////////////////////
// Init SDL screen and Font
// //////////////////////////////////////////////
int init_sdl(TTF_Font **font, SDL_Window **window, SDL_Renderer **renderer)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 0;
    }

    if (TTF_Init() != 0) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 0;
    }

    *window = SDL_CreateWindow("SDL Client",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               400, 200, 0);

    if (!*window) {
        printf("Window Error: %s\n", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {
        printf("Renderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    int text_size = 200;
    *font = TTF_OpenFont("HackNerdFontMono-Regular.ttf", text_size);

    if (!*font) {
        printf("Font Error: %s\n", TTF_GetError());
        SDL_DestroyRenderer(*renderer);
        SDL_DestroyWindow(*window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    return 1;
}


