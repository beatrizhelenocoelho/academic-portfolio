#ifndef STRUCTS_H
#define STRUCTS_H


#include <pthread.h>
#include <SDL2/SDL.h>

extern int player_radius;
extern int trash_radius;

// Structs to save all planets information
typedef struct planet_structure {

    char name [2];
    int x, y;
    int trash;
    int mass;
    SDL_Color *color;

}planet_structure;


//Vector info used on Universe-simulator
typedef struct vector{
    
    float amplitude;
    float angle;

}vector;


// Struct to save trash info
typedef struct trash_structure{
    int x, y;
    int mass;
    vector acceleration;
    vector velocity;
    SDL_Color *color;
    int taken;
    char ship_name[2];

}trash_structure;

/*
//Ship info needed
typedef struct ship{
    float x, y;
    char name[2];
    int pass;
    int use;
    int trash;

}ship;
*/

typedef struct ship{
    float x, y;
    char name[2];

    vector acceleration;
    vector velocity;
    
    int active;
    int trash;      // lixo transportado
    int pass;
    int use;
} ship;


//ALL direction traduction from 
//movement to number for message
typedef enum {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3
} direction_t;

//Tradution from result 
// to message
typedef enum {
    RES_OK = 0,
    RES_ERROR = 1
} respost;

#endif
