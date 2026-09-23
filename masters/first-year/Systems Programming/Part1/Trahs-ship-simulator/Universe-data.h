#ifndef UNIVERSE_DATA_H
#define UNIVERSE_DATA_H

#include <SDL2/SDL.h>
#include "structs.h"

// ============================================================================
// Create planet array
// ----------------------------------------------------------------------------
// Creates `n_planets` planets with random positions inside the window area.
// One planet is assigned the recycling color (`recicling`).
//
// Parameters:
//   n_planets   - number of planets to generate
//   window_size - window dimension used for valid random positions
//   mass        - default mass assigned to each planet
//   main        - color used for all normal planets
//   recicling   - special color used to mark the recycling planet
//
// Returns:
//   Pointer to dynamically allocated planet array (size = n_planets).
// ============================================================================
planet_structure *create_planet(int n_planets, int window_size, int mass,
                                SDL_Color *main, SDL_Color *recicling);

// ============================================================================
// Create initial trash array
// ----------------------------------------------------------------------------
// Allocates total `num_trash` trash items and initializes the first
// `num_trash_init` items. Remaining items are marked unused.
//
// Parameters:
//   color           - trash color
//   mass            - default mass of trash
//   window_size     - window dimension for random placement
//   num_trash_init  - number of trash pieces to initially spawn
//   num_trash       - total size of allocated trash array
//
// Returns:
//   Pointer to dynamically allocated trash array.
// ============================================================================
trash_structure *create_trash_init(SDL_Color *color, int mass, int window_size,
                                   int num_trash_init, int num_trash);

// ============================================================================
// Initialize ships array
// ----------------------------------------------------------------------------
// Creates `n_ships` ships with random positions, generated names, and defaults.
//
// Parameters:
//   n_ships      - number of ships to create
//   window_size  - window dimension for random position generation
//
// Returns:
//   Pointer to dynamically allocated ship array.
// ============================================================================
ship *ship_init(int n_ships, int window_size);

// ============================================================================
// Find next available ship
// ----------------------------------------------------------------------------
// Locates the first ship whose `use == 0` and marks it active.
//
// Parameters:
//   ships    - array of ships
//   n_ships  - number of ships in the array
//
// Returns:
//   Index of the activated ship, or -1 if none available.
// ============================================================================
int next_ship_use(ship *ships, int n_ships);

// ============================================================================
// Spill trash (drop carried trash)
// ----------------------------------------------------------------------------
// Drops all trash carried by ships into random positions on the map.
//
// Parameters:
//   ships       - array of ships
//   trash       - array of trash objects
//   n_trash     - total number of trash objects
//   window_size - window dimension for placing dropped trash
// ============================================================================
void spiled_trash(ship *ships, trash_structure *trash,
                  int n_trash, int window_size);

// ============================================================================
// Recycle trash held by ships
// ----------------------------------------------------------------------------
// Marks all trash carried by a ship as recycled and resets its cargo.
//
// Parameters:
//   ships    - array of ships
//   trash    - array of trash objects
//   n_trash  - total number of trash objects
// ============================================================================
void recicle_trash(ship *ships, trash_structure *trash, int n_trash);

#endif // UNIVERSE_DATA_H

