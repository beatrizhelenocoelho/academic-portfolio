#ifndef PLAYER_H
#define PLAYER_H

#include "structs.h"
#include <SDL2/SDL.h>

// ============================================================================
// Check ship information
// ----------------------------------------------------------------------------
// Compares a received ship name and password against saved ships.
//
// Parameters:
//   ships   - array of ships
//   n_ships - number of ships in array
//   c       - ship's name character received from message
//   pass    - password received from message
//
// Returns:
//   Index of matching ship in array, or -1 if not found.
// ============================================================================
int check_info(ship *ships, int n_ships, char c, int pass);

// ============================================================================
// Calculate new ship position
// ----------------------------------------------------------------------------
// Computes new (x,y) coordinates based on direction and step, 
// ensuring ship stays inside the window bounds.
//
// Parameters:
//   x           - pointer to current x-coordinate (will be updated)
//   y           - pointer to current y-coordinate (will be updated)
//   direction   - movement direction (enum direction_t)
//   WINDOW_SIZE - size of the square window
//   step        - number of pixels to move
//
// Returns:
//   1 if movement is valid, 0 if blocked by boundaries.
// ============================================================================
int new_position(int* x, int *y, direction_t direction, int WINDOW_SIZE, int step);

// ============================================================================
// Handle collisions and interactions
// ----------------------------------------------------------------------------
// Checks if ship hits trash or planet. Collects trash or deposits it.
//
// Parameters:
//   ships     - array of ships
//   trash     - array of trash objects
//   curr_trash- index of the current ship
//   n_planets - number of planets
//   planets   - array of planets
//   recicling - color of recycling planet
//   margin    - radius around planet for deposit detection
//
// Returns:
//   1 if collision/deposit occurred, 0 otherwise.
// ============================================================================
int hit(ship *ships, trash_structure * trash, int curr_trash, int n_planets,
        planet_structure *planets, SDL_Color *recicling, int margin);

#endif 

