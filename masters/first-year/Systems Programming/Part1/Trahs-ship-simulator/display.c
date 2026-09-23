
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "SDL2/SDL_pixels.h"
#include <time.h>
#include <SDL2/SDL_ttf.h>
#include <libconfig.h>
#include "SDL2/SDL2_gfxPrimitives.h"
#include "structs.h"

/////////////////////////////////////////
///Sets renderer draw color using SDL_Color directly
/////////////////////////////////////////
void SDL_SetRenderDrawColor_Direct_Color(SDL_Renderer *renderer, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

/////////////////////////////////////////
///Initializes SDL window and renderer with background color
/////////////////////////////////////////
int initialize_SDL_simple(int window_size, SDL_Window **window, SDL_Renderer **renderer, SDL_Color color)
{
   if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 0;
    }

    *window = SDL_CreateWindow("Universe-Simulator",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   window_size ,
                                   window_size ,
                                   0);
    if (!*window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 0;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return 0;
    }
    

    SDL_SetRenderDrawColor_Direct_Color(*renderer,color);  // black
    SDL_RenderClear(*renderer);
    return 1; // success
}



/////////////////////////////////////////
///Initializes SDL_ttf and loads font file
/////////////////////////////////////////
int initialize_SDL_font(TTF_Font** font, int text_size, const char *font_path){
    if (TTF_Init() != 0) {
        printf("TTF_Init Error: %s\n", TTF_GetError());
        SDL_Quit();
        return 1;
    }

    *font = TTF_OpenFont(font_path, text_size);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    return 0;

}


////// ==================== //////////////
// Convert SDL_Color to the format required by SDL2_gfx
////// ==================== //////////////
Uint32 SDL_ColorToUint(SDL_Color *c){
	return (Uint32)((c->a << 24) + (c->b << 16) + (c->g << 8)+ (c->r << 0));
}

/////////////////////////////////////////
///Draws planets and labels using SDL2_gfx and font
/////////////////////////////////////////
void draw_planet(SDL_Renderer *renderer, planet_structure *planets, int num_planets, int cell_size, TTF_Font *font)
{
    char display_string[32];
    for(int i = 0; i <  num_planets; i++){
        int center_x = planets[i].x  ;
        int center_y = planets[i].y ;

        snprintf(display_string, sizeof(display_string), "%s %d", planets[i].name, planets[i].trash);

        Uint32 color_uint = SDL_ColorToUint(planets[i].color);

        // 4. Draw the filled circle using SDL2_gfx
        // filledCircleColor(SDL_Renderer *renderer, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
        filledCircleColor(renderer, (Sint16)center_x, (Sint16)center_y, (Sint16)cell_size, color_uint);

         SDL_Surface* textSurface = TTF_RenderText_Solid(font, display_string, *(planets[i].color));
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                SDL_FreeSurface(textSurface);

                SDL_Rect dst = { center_x + 10 , center_y + 10 , 0, 0 };
                SDL_QueryTexture(textTexture, NULL, NULL, &dst.w, &dst.h);
                SDL_RenderCopy(renderer, textTexture, NULL, &dst);
                SDL_DestroyTexture(textTexture);


    }
}

/////////////////////////////////////////
///Draws untaken trash circles on screen
/////////////////////////////////////////
void draw_trash(SDL_Renderer *renderer, trash_structure *trash, int num_trash, int trash_size){
  
    
    Uint32 color_uint = SDL_ColorToUint(trash[0].color);

    for (int i = 0; i < num_trash; i ++){
        if(trash[i].taken == 0){
        int center_x = trash[i].x;
        int center_y = trash[i].y;

    filledCircleColor(renderer, (Sint16)center_x, (Sint16)center_y, (Sint16)trash_size, color_uint);
        }
    }

}


/////////////////////////////////////////
///Draws active ships and labels on thoses
/////////////////////////////////////////
void draw_ships(SDL_Renderer *renderer, ship *ships, int num_planets, int cell_size, TTF_Font *font, SDL_Color * color)
{
    char display_string[32];
    for(int i = 0; i <  num_planets; i++){
        if(ships[i].use == 1){
        int center_x = ships[i].x  ;
        int center_y = ships[i].y ;

        snprintf(display_string, sizeof(display_string), "%s %d", ships[i].name, ships[i].trash);

        Uint32 color_uint = SDL_ColorToUint(color);

        // 4. Draw the filled circle using SDL2_gfx
        // filledCircleColor(SDL_Renderer *renderer, Sint16 x, Sint16 y, Sint16 r, Uint32 color);
        filledCircleColor(renderer, (Sint16)center_x, (Sint16)center_y, (Sint16)cell_size, color_uint);

         SDL_Surface* textSurface = TTF_RenderText_Solid(font, display_string, *(color));
                SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
                SDL_FreeSurface(textSurface);

                SDL_Rect dst = { center_x + 10 , center_y + 10 , 0, 0 };
                SDL_QueryTexture(textTexture, NULL, NULL, &dst.w, &dst.h);
                SDL_RenderCopy(renderer, textTexture, NULL, &dst);
                SDL_DestroyTexture(textTexture);

        }
    }
}


/////////////////////////////////////////
///Clears screen and draws planets, trash, and ships
/////////////////////////////////////////
void render_all(SDL_Renderer *renderer,
                planet_structure *planets, int n_planets, int planet_radius,
                trash_structure *trash, int n_trash, int trash_radius,ship *ships,
                int n_ships, TTF_Font *font, SDL_Color bg, SDL_Color c_ships)
{
    SDL_SetRenderDrawColor(renderer, bg.r, bg.g, bg.b, bg.a);
    SDL_RenderClear(renderer);

    draw_planet(renderer, planets, n_planets, planet_radius, font);
    draw_trash(renderer, trash, n_trash, trash_radius);
    draw_ships(renderer, ships, n_ships, planet_radius, font, &c_ships);

    SDL_RenderPresent(renderer);
}

