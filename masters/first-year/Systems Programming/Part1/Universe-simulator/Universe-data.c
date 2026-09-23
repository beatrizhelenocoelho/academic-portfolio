#include "structs.h"
#include <SDL2/SDL_pixels.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>   // for rand(), srand()
#include <time.h>     // for time()
#include <SDL2/SDL.h>



planet_structure *create_planet(int n_planets, int window_size, int mass, SDL_Color *main, SDL_Color *recicling ){
    
    planet_structure *planets = malloc(n_planets * sizeof(planet_structure));
    if (!planets){
        return NULL;
    }
    for (int i = 0; i < n_planets; i ++){
        planets[i].x = rand() % window_size;
        planets[i].y = rand() % window_size;
        planets[i].color = main;
        planets[i].mass  = mass;
        planets[i].trash = 0;

        planets[i].name[0] = 'A' + (i % 26); // wraps around after 'Z'
        planets[i].name[1] = '\0';           // null terminator
    }
    
    int recicle = rand() %n_planets;
    planets[recicle].color = recicling;

    return planets;


}

trash_structure create_trash(SDL_Color *color ,int mass, int window_size)
{
    trash_structure trash;
    trash.x = rand() % window_size;
    trash.y = rand() % window_size;
    trash.mass = mass;
    trash.color = color;

    trash.acceleration.amplitude = 0.0f;
    trash.acceleration.angle     = 0.0f;
    trash.velocity.amplitude     = 0.0f;
    trash.velocity.angle         = 0.0f;

    return trash;
}


trash_structure *create_trash_init(SDL_Color *color, int mass,
                                   int window_size, int num_trash, int max_trash)
{
    // alocamos espaço para TODO o lixo possível (até max_trash)
    trash_structure *trash_array = malloc(max_trash * sizeof(trash_structure));
    if (trash_array == NULL) {
        return NULL;
    }

    // inicializamos só os primeiros num_trash elementos
    for (int i = 0; i < num_trash; i++) {
        trash_array[i] = create_trash(color, mass, window_size);
    }

    for (int i = num_trash; i < max_trash; i++) {
        trash_array[i].x = 0;
        trash_array[i].y = 0;
        trash_array[i].mass = mass;
        trash_array[i].color = color;
        trash_array[i].acceleration.amplitude = 0.0f;
        trash_array[i].acceleration.angle     = 0.0f;
        trash_array[i].velocity.amplitude     = 0.0f;
        trash_array[i].velocity.angle         = 0.0f;
    }

    return trash_array;
}

