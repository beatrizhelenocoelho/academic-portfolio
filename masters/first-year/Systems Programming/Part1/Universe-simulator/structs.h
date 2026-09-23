#ifndef STRUCTS_H
#define STRUCTS_H



#include <SDL2/SDL.h>

typedef struct planet_structure {

    char name [2];
    int x, y;
    int trash;
    int mass;
    SDL_Color *color;

}planet_structure;

typedef struct vector{
    
    float amplitude;
    float angle;

}vector;

typedef struct trash_structure{
    int x, y;
    int mass;
    vector acceleration;   
    vector velocity;
    SDL_Color *color;

} trash_structure;

void set_universe_size(int size);
void new_trash_acceleration(planet_structure planets[], int total_planets,
                            trash_structure trash[], int total_trash);
void new_trash_velocity(trash_structure trash[], int total_trash);
void new_trash_position(trash_structure trash[], int total_trash);


#endif
