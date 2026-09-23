#include "structs.h"
#include <SDL2/SDL_pixels.h>
#include <stdlib.h>  
#include <time.h>     
#include <SDL2/SDL.h>

//////////////////////////////
///Creates all planets with random position, and selects recicling one 
//////////////////////////////

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

//////////////////////////////
///Creates one trash object with random position to start movement
//////////////////////////////
trash_structure create_trash(SDL_Color *color ,int mass, int window_size)
{
    
    trash_structure trash;
    trash.x = rand() %window_size;
    trash.y = rand() %window_size;
    trash.mass = mass;
    trash.color = color;
    trash.taken = 0;

    return trash;
}

//////////////////////////////
///Allocates trash array and initializes all initial trash
//////////////////////////////
trash_structure *create_trash_init(SDL_Color *color, int mass, int window_size, int num_trash_init, int num_trash)
{
    trash_structure *trash = malloc(num_trash * sizeof(trash_structure));
    for (int i = 0; i <num_trash_init; i++){
        trash[i] = create_trash(color, mass, window_size);
        

    }
    return trash;
}
//////////////////////////////
///Creates ships with random positions, names
//////////////////////////////
ship *ship_init(int n_ships, int window_size){
    ship *ships= malloc(n_ships * sizeof(ship));
    for (int i = 0; i < n_ships; i ++){
        ships[i].x = rand() % window_size;
        ships[i].y = rand() % window_size;


        ships[i].name[0] = 'A' + (i % 26); // wraps around after 'Z'
        ships[i].name[1] = '\0';
        ships[i].trash = 0;
        ships[i].use = 0;
        ships[i].pass = rand()%256;
    }
    return ships;

}
//////////////////////////////
///Finds first unused ship and marks it active
//////////////////////////////
int next_ship_use(ship *ships, int n_ships){
    for (int i = 0; i <n_ships; i ++){
        if(ships[i].use == 0){
            ships[i].use = 1;
            return i;
        }
    }
    return -1;

}
//////////////////////////////
///Drops carried trash in randomizes places
//////////////////////////////
void spiled_trash( ship*ships, trash_structure *trash, int n_trash, int window_size){
    
    for (int i =0; i < n_trash; i++){
        if (strcmp(trash[i].ship_name,ships->name) == 0 && trash[i].taken == 1){
                trash[i].taken = 0;
                trash[i].x = rand() %window_size;
        
                trash[i].y = rand() %window_size;
                ships->trash --;
        }
    }

}
//////////////////////////////
///Marks carried trash as recycled on a specific ship
//////////////////////////////
void recicle_trash( ship*ships, trash_structure *trash, int n_trash){
    
    for (int i =0; i < n_trash; i++){
    if (strcmp(trash[i].ship_name,ships->name) == 0 && trash[i].taken == 1){
            trash[i].taken = 2;
    }
    }

}
