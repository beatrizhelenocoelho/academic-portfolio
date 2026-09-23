#include <stdio.h>
#include "structs.h"


// To find if ship and name recive from message coincide
// from saved and if so return so idx
int check_info(ship *ships, int n_ships, char c, int pass)
{
    for (int i = 0; i < n_ships; i++) {

        // ship name is 1 char + '\0'
        if (ships[i].name[0] == c && ships[i].pass == pass && ships[i].use == 1) {
            return i;   // match found
        }
    }

    return -1; // no match
}

//To get new position of ship on map for ship
int new_position(int* x, int *y, direction_t direction, int WINDOW_SIZE, int step)
{
    int hit_wall = 0;
   

    switch (direction)
    {
    case UP:
        *x -= step;
        if (*x < 0) {
            *x = WINDOW_SIZE + *x;   // go from top to bottom
        }
        break;

    case DOWN:
        *x += step;
        if (*x >= WINDOW_SIZE) {
            *x = WINDOW_SIZE - *x;  //go from bottom to top
        }
        break;

    case LEFT:
        *y -= step;
        if (*y < 0) {
            *y = WINDOW_SIZE + *y;  //go left to right 
        }
        break;

    case RIGHT:
        *y += step;
        if (*y >= WINDOW_SIZE) {
            *y = WINDOW_SIZE - *y; //go right top left
        }
        break;

    default:
            hit_wall = 1;
        break;
    }

    return hit_wall;
}

//To compare two SDL colors
int same_color(SDL_Color *a, SDL_Color *b) {
    return (a->r == b->r &&
            a->g == b->g &&
            a->b == b->b &&
            a->a == b->a);
}


//Check if hit either trash or a planet 
// if trash takes it
//if planet is the recling one deposit trash
int hit(ship *ships, trash_structure *trash, int curr_trash,
        int n_planets, planet_structure *planets, SDL_Color *recicling ,int margin)
{
                                            
    for (int i = 0; i < curr_trash; i++) {                  //loop to check trash hit
        if (abs(ships->x - trash[i].x) <= margin &&
            abs(ships->y - trash[i].y) <= margin &&
            trash[i].taken == 0)
        {
            ships->trash++;
            trash[i].taken = 1;
            printf("HIT\n");
            strcpy(trash[i].ship_name, ships->name);
            return 1;
        }
    }

    for (int i = 0; i < n_planets; i++) {               //loop to chech planet hit
        if (abs(ships->x - planets[i].x) <= margin &&
            abs(ships->y - planets[i].y) <= margin)
        {
            if (same_color(planets[i].color, recicling)) {
                printf("RECICLING\n");
                planets[i].trash += ships->trash;
                ships->trash = 0;
                return 2;
            }else{
                return 3;
            
            

            }
        }
    }
    return 0;
}

