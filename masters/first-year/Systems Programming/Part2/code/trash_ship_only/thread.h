#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <stddef.h>   // for size_t
#include <SDL2/SDL.h>
#include "structs.h"

// ============================================================================
// GLOBAL SYNCHRONIZATION
// ----------------------------------------------------------------------------
// Mutex shared across client modules to protect access to the world state.
// Must be locked before reading or writing any shared world data.
// ============================================================================

extern pthread_mutex_t world_mutex;

// ============================================================================
// SUBSCRIBER THREAD ARGUMENTS
// ----------------------------------------------------------------------------
// Structure passed to the subscriber thread, containing all references needed
// to safely update the client-side world state.
// ============================================================================

typedef struct {
    void *fd_sub;                   // ZMQ SUB socket used to receive world updates

    planet_structure **planets;     // Pointer to current planet array
    trash_structure  **trash;       // Pointer to current trash array
    ship             **ships;       // Pointer to current ships array

    size_t *n_planets;              // Number of planets currently loaded
    size_t *n_trash;                // Number of trash objects
    size_t *n_ships;                // Number of ships

    int *running;                   // Thread control flag (0 = stop)
    int *has_world;                 // World state flag (ready / error codes)

    Uint32 world_update;            // SDL event type used to notify main loop
} sub_thread_args;

// ============================================================================
// WORLD EVENT CODES
// ----------------------------------------------------------------------------
// Custom event codes used to communicate world state changes from the
// subscriber thread to the SDL main loop.
// ============================================================================

typedef enum {
    WORLD_EVENT_UPDATE        = 1,  // New world state received
    WORLD_SERVER_DOWN         = 2,  // Server became unreachable
    WORLD_EVENT_SERVER_CLOSED = 5,  // Server closed the world gracefully
    WORLD_EVENT_COLAPSED      = 6   // Universe collapse condition reached
} world_event_code;

// ============================================================================
// MEMORY MANAGEMENT
// ----------------------------------------------------------------------------
// Utility functions related to dynamic allocation of world state.
// ============================================================================

/**
 * Frees all dynamically allocated world data.
 *
 * Releases:
 *  - Planet array and associated colors
 *  - Trash array and associated colors
 *  - Ship array
 *
 * Must NOT be called while another thread is accessing the world.
 *
 * @param planets   Array of planets.
 * @param n_planets Number of planets.
 * @param trash     Array of trash objects.
 * @param n_trash   Number of trash objects.
 * @param ships     Array of ships.
 */
void free_world(planet_structure *planets, size_t n_planets,
                trash_structure *trash, size_t n_trash,
                ship *ships);

// ============================================================================
// THREAD ENTRY POINTS
// ----------------------------------------------------------------------------
// Functions executed by pthread_create.
// ============================================================================

/**
 * Subscriber thread function.
 *
 * Continuously receives world state updates from the server via ZeroMQ,
 * replaces the local world representation atomically, and notifies the
 * SDL main loop using custom SDL events.
 *
 * @param arg Pointer to a sub_thread_args structure.
 * @return NULL when the thread terminates.
 */
void *sub_thread_func(void *arg);

#endif /* THREAD_H */
