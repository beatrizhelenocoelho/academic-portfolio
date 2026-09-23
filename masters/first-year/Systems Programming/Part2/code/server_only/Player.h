#ifndef PLAYER_H
#define PLAYER_H

#include "structs.h"
#include <SDL2/SDL.h>

// ============================================================================
// Ship identification and validation
// ----------------------------------------------------------------------------
// Validates a ship based on the identifier and password received from a client.
// Only ships that are currently active (use == 1) are considered valid.
// ============================================================================

/**
 * Checks if the received ship information matches an active ship.
 *
 * Compares the ship name character and password against the stored ships.
 *
 * @param ships   Array of ships.
 * @param n_ships Number of ships in the array.
 * @param c       Ship identifier character received from the message.
 * @param pass    Password received from the message.
 *
 * @return Index of the matching ship if found, or -1 if no valid match exists.
 */
int check_info(ship *ships, int n_ships, char c, int pass);

// ============================================================================
// Ship movement (step-based, legacy)
// ----------------------------------------------------------------------------
// Computes the new position of a ship using discrete movement steps.
// Implements wrap-around behavior when crossing universe boundaries.
// ============================================================================

/**
 * Calculates a new ship position based on movement direction.
 *
 * The ship moves a fixed number of pixels (step) in the given direction.
 * If the ship exits the window boundaries, it re-enters from the opposite side.
 *
 * @param x           Pointer to the current x-coordinate (updated in place).
 * @param y           Pointer to the current y-coordinate (updated in place).
 * @param direction   Direction of movement.
 * @param WINDOW_SIZE Size of the square window.
 * @param step        Number of pixels to move per step.
 *
 * @return 0 if the movement is valid,
 *         1 if the direction is invalid.
 */
int new_position(int* x, int *y, direction_t direction,
                 int WINDOW_SIZE, int step);

// ============================================================================
// Collision detection and interaction handling
// ----------------------------------------------------------------------------
// Detects collisions between a ship and trash or planets and performs the
// corresponding game logic (collection, recycling, or spilling).
// ============================================================================

/**
 * Checks collisions between a ship and trash or planets.
 *
 * Possible interactions:
 *  - Collect trash (if ship is not full)
 *  - Deposit trash in recycling planet
 *  - Spill trash in a non-recycling planet
 *
 * @param ships      Pointer to the ship being checked.
 * @param trash      Array of trash objects.
 * @param curr_trash Number of trash objects currently in the world.
 * @param n_planets  Number of planets.
 * @param planets    Array of planets.
 * @param recicling  Color identifying the recycling planet.
 * @param margin     Collision margin for planet interaction.
 *
 * @return >= 0  Index of the trash collected
 *         -1    No relevant collision
 *         -2    Collision with recycling planet
 *         -3    Collision with spill planet
 */
int hit(ship *ships,
        trash_structure *trash, int curr_trash,
        int n_planets, planet_structure *planets,
        SDL_Color *recicling, int margin);

#endif /* PLAYER_H */
