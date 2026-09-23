#ifndef PLANETS_H
#define PLANETS_H

#include "structs.h"

planet_structure *create_planet(int n_planets, int window_size,
                                int mass, SDL_Color *main, SDL_Color *recicling);

trash_structure *create_trash_init(SDL_Color *color, int mass,
                                   int window_size, int num_trash, int max_trash);

trash_structure  create_trash(SDL_Color *color ,int mass, int window_size);

#endif
