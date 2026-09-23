#ifndef UNIVERSE_DATA_H
#define UNIVERSE_DATA_H

#include <SDL2/SDL.h>
#include "structs.h"

// ============================================================================
// Planet creation
// ----------------------------------------------------------------------------
// Functions responsible for creating and initializing planets in the universe.
// ============================================================================

/**
 * Creates and initializes an array of planets.
 *
 * All planets are created with random positions inside the universe window.
 * One planet is randomly selected to act as the recycling planet and is
 * identified by a different color.
 *
 * @param n_planets   Number of planets to create.
 * @param window_size Size of the universe (used for random positions).
 * @param mass        Default mass assigned to each planet.
 * @param main        Color assigned to normal planets.
 * @param recicling   Color used to identify the recycling planet.
 *
 * @return Pointer to a dynamically allocated array of planets,
 *         or NULL if memory allocation fails.
 */
planet_structure *create_planet(int n_planets,
                                int window_size,
                                int mass,
                                SDL_Color *main,
                                SDL_Color *recicling);

// ============================================================================
// Trash creation
// ----------------------------------------------------------------------------
// Functions responsible for creating and initializing trash objects.
// ============================================================================

/**
 * Creates a single trash object with random position.
 *
 * The trash starts as not collected (taken = 0).
 *
 * @param color        Color of the trash.
 * @param mass         Mass of the trash.
 * @param window_size  Size of the universe (used for random positions).
 *
 * @return Initialized trash_structure.
 */
trash_structure create_trash(SDL_Color *color, int mass, int window_size);

/**
 * Allocates and initializes the trash array.
 *
 * The array is allocated with capacity for num_trash elements, but only
 * the first num_trash_init elements are initialized at creation time.
 *
 * @param color           Color of the trash.
 * @param mass            Default mass of trash objects.
 * @param window_size     Size of the universe.
 * @param num_trash_init  Number of trash objects initially created.
 * @param num_trash       Total allocated size of the trash array.
 *
 * @return Pointer to a dynamically allocated trash array.
 */
trash_structure *create_trash_init(SDL_Color *color,
                                   int mass,
                                   int window_size,
                                   int num_trash_init,
                                   int num_trash);

// ============================================================================
// Ship creation and management
// ----------------------------------------------------------------------------
// Functions responsible for creating ships and managing their availability.
// ============================================================================

/**
 * Creates and initializes an array of ships.
 *
 * Ships are created with random positions and generated names.
 * All ships start inactive (use = 0) with empty cargo.
 *
 * @param n_ships      Number of ships to create.
 * @param window_size  Size of the universe (used for random positions).
 *
 * @return Pointer to a dynamically allocated array of ships.
 */
ship *ship_init(int n_ships, int window_size);

/**
 * Finds the first available (inactive) ship and marks it as active.
 *
 * @param ships    Array of ships.
 * @param n_ships  Number of ships in the array.
 *
 * @return Index of the activated ship, or -1 if none are available.
 */
int next_ship_use(ship *ships, int n_ships);

// ============================================================================
// Trash interaction
// ----------------------------------------------------------------------------
// Functions that handle trash dropping and recycling logic.
// ============================================================================

/**
 * Spills all trash carried by a ship into the universe.
 *
 * The trash is dropped at random positions until either the ship cargo
 * is empty or the maximum allowed trash limit is reached.
 *
 * @param ship          Pointer to the ship spilling trash.
 * @param trash         Array of trash objects.
 * @param current_trash Pointer to the current number of trash objects.
 * @param max_trash     Maximum allowed trash in the universe.
 * @param window_size   Size of the universe.
 * @param trash_color   Color assigned to spilled trash.
 * @param was_colliding Collision state array used by the physics system.
 */
void spiled_trash(ship *ship,
                  trash_structure *trash,
                  int *current_trash,
                  int max_trash,
                  int window_size,
                  SDL_Color *trash_color,
                  int *was_colliding);

/**
 * Marks trash collected by a specific ship as recycled.
 *
 * Trash objects are identified by the ship name stored at collection time.
 *
 * @param ships   Pointer to the ship recycling trash.
 * @param trash   Array of trash objects.
 * @param n_trash Number of trash objects.
 */
void recicle_trash(ship *ships,
                   trash_structure *trash,
                   int n_trash);

// ============================================================================
// Ship activity check
// ----------------------------------------------------------------------------
// Utility function used to detect if at least one ship is active.
// ============================================================================

/**
 * Checks if there is at least one active ship in the universe.
 *
 * @param ships   Array of ships.
 * @param n_ships Number of ships.
 *
 * @return 1 if at least one ship is active, 0 otherwise.
 */
int has_active_ship(ship *ships, int n_ships);

#endif /* UNIVERSE_DATA_H */
