#ifndef GRAVITATION_H
#define GRAVITATION_H

#include "structs.h"

// ============================================================================
// Universe physics configuration
// ----------------------------------------------------------------------------
// Defines global parameters that affect the physical simulation of the universe,
// such as its size and boundary behavior.
// ============================================================================

/**
 * Sets the size of the universe.
 *
 * The universe uses a wrap-around (toroidal) model: when an object exits one
 * side of the universe, it re-enters from the opposite side.
 *
 * This function must be called before any position correction or movement
 * calculations are performed.
 *
 * @param size  Size of the universe (both width and height).
 */
void set_universe_size(int size);

// ============================================================================
// Trash physics
// ----------------------------------------------------------------------------
// Functions responsible for computing gravitational effects, velocity updates
// and position updates of trash elements in the universe.
// ============================================================================

/**
 * Computes the gravitational acceleration applied to each trash element.
 *
 * Each trash object is attracted by all planets according to a simplified
 * gravitational model. The resulting acceleration vector is the sum of all
 * individual gravitational forces.
 *
 * @param planets        Array of planets exerting gravitational force.
 * @param total_planets  Number of planets.
 * @param trash          Array of trash objects.
 * @param total_trash    Number of trash objects.
 */
void new_trash_acceleration(planet_structure planets[], int total_planets,
                            trash_structure trash[], int total_trash);

/**
 * Updates the velocity of each trash object.
 *
 * Applies a small friction factor to simulate space resistance and then
 * adds the current acceleration vector.
 *
 * @param trash        Array of trash objects.
 * @param total_trash  Number of trash objects.
 */
void new_trash_velocity(trash_structure trash[], int total_trash);

/**
 * Updates the position of each trash object.
 *
 * The new position is computed from the current velocity and corrected
 * according to the universe boundaries (wrap-around behavior).
 *
 * @param trash        Array of trash objects.
 * @param total_trash  Number of trash objects.
 */
void new_trash_position(trash_structure trash[], int total_trash);

// ============================================================================
// Ship physics
// ----------------------------------------------------------------------------
// Functions responsible for gravitational interaction, movement and manual
// thrust control of ships.
// ============================================================================

/**
 * Computes the gravitational acceleration applied to each ship.
 *
 * Ships are attracted by all planets. Ship mass is considered constant
 * (mass = 1), as defined in the project specification.
 *
 * @param planets        Array of planets exerting gravitational force.
 * @param total_planets  Number of planets.
 * @param ships          Array of ships.
 * @param total_ships    Number of ships.
 */
void new_ship_acceleration(planet_structure planets[], int total_planets,
                           ship ships[], int total_ships);

/**
 * Updates the velocity of each ship.
 *
 * Applies friction and adds the computed acceleration vector.
 *
 * @param ships        Array of ships.
 * @param total_ships  Number of ships.
 */
void new_ship_velocity(ship ships[], int total_ships);

/**
 * Updates the position of each ship.
 *
 * Uses the current velocity to compute the new position and applies
 * universe boundary correction (wrap-around).
 *
 * @param ships        Array of ships.
 * @param total_ships  Number of ships.
 */
void new_ship_position(ship ships[], int total_ships);

/**
 * Applies a manual thrust to a ship.
 *
 * Adds a small velocity vector in the given direction, allowing player
 * control over ship movement.
 *
 * @param s  Pointer to the ship to apply thrust to.
 * @param d  Direction of thrust (UP, DOWN, LEFT, RIGHT).
 */
void new_ship_thrust(ship *s, direction_t d);

#endif /* GRAVITATION_H */

