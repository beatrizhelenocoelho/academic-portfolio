#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL_ttf.h>
#include <libconfig.h>
#include "structs.h"


int initialize_SDL_simple(int window_size, SDL_Window **window, SDL_Renderer **renderer,  SDL_Color color);
int initialize_SDL_font(TTF_Font** font, int text_size, const char *font_path); 
void SDL_SetRenderDrawColor_Direct_Color(SDL_Renderer *renderer, SDL_Color color); 
void draw_planet(SDL_Renderer *renderer, planet_structure *planets, int num_planets, int cell_size, TTF_Font *font);
void draw_trash(SDL_Renderer *renderer, trash_structure *trash, int num_trash, int trash_size);

#endif
