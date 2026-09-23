#ifndef THREADS_SERVER_H
#define THREADS_SERVER_H

#include <pthread.h>
#include <SDL2/SDL.h>
#include "structs.h"

// ============================================================================
// GLOBAL SHARED STATE (SERVER SIDE)
// ----------------------------------------------------------------------------
// This header centralizes all global variables shared between server threads.
// Access to these variables MUST be protected using the world_mutex to avoid
// race conditions.
// ============================================================================

/* ---------------------------------------------------------------------------
   World objects
   ---------------------------------------------------------------------------
   Arrays representing the current state of the universe.
   --------------------------------------------------------------------------- */
extern planet_structure *planets;     // Array of planets
extern trash_structure  *trash;       // Array of trash objects
extern ship             *ships;       // Array of ships

/* ---------------------------------------------------------------------------
   World configuration
   --------------------------------------------------------------------------- */
extern int n_planets;                 // Total number of planets
extern int initial_trash;             // Initial number of trash objects
extern int max_trash;                 // Maximum allowed trash before collapse
extern int trash_radius;              // Collision radius for trash
extern int window_size;               // Universe/window size
extern int player_radius;             // Collision radius for ships
extern int planet_radius;             // Collision radius for planets

/* ---------------------------------------------------------------------------
   Ship activity tracking
   ---------------------------------------------------------------------------
   ship_active[i] indicates if ship i has recently sent an ACTIVE message.
   Used to detect disconnected or inactive players.
   --------------------------------------------------------------------------- */
extern int *ship_active;

/* ---------------------------------------------------------------------------
   Dynamic world state
   --------------------------------------------------------------------------- */
extern int current_trash;             // Current number of trash objects
extern int quit;                      // Global quit flag
extern int universe_collapsed;        // Indicates universe collapse condition

/* ---------------------------------------------------------------------------
   Collision state tracking
   ---------------------------------------------------------------------------
   Used to detect collision transitions (entering vs continuous collision),
   allowing controlled creation of new trash.
   --------------------------------------------------------------------------- */
extern int *was_colliding;

/* ---------------------------------------------------------------------------
   Synchronization
   ---------------------------------------------------------------------------
   Mutex protecting all shared world state.
   MUST be locked before reading or writing any shared variable above.
   --------------------------------------------------------------------------- */
extern pthread_mutex_t world_mutex;

/* ---------------------------------------------------------------------------
   Network configuration
   --------------------------------------------------------------------------- */
extern const char *port;              // Server main communication port
extern const char *port_sub;          // Server publish/subscribe port

/* ---------------------------------------------------------------------------
   Common colors
   ---------------------------------------------------------------------------
   Shared SDL_Color definitions used across server modules.
   --------------------------------------------------------------------------- */
extern SDL_Color red;
extern SDL_Color blue;
extern SDL_Color green;
extern SDL_Color bg0;
extern SDL_Color black;

// ============================================================================
// NETWORK THREAD CONTEXT
// ----------------------------------------------------------------------------
// Context structure passed to server threads containing thread-specific
// configuration and communication handles.
// ============================================================================

typedef struct {
    void *fd;                 // ZMQ socket for request/reply communication
    void *pub;                // ZMQ publisher socket
    int  *running;            // Pointer to global running flag

    int window_size;          // Cached universe/window size
    int step;                 // Movement step (legacy / discrete movement)
    int margin;               // Collision margin for planets

    SDL_Color *planet_color;  // Color identifying the recycling planet
} net_thread_ctx;

// ============================================================================
// THREAD ENTRY POINTS
// ----------------------------------------------------------------------------
// Thread functions executed by pthread_create.
// ============================================================================

/**
 * Network thread.
 *
 * Responsible for:
 *  - Receiving client messages
 *  - Updating ship state based on commands
 *  - Sending responses to clients
 *
 * @param arg Pointer to a net_thread_ctx structure.
 * @return NULL when thread terminates.
 */
void *network_thread(void *arg);

/**
 * Physics thread.
 *
 * Responsible for:
 *  - Periodic physics updates
 *  - Gravitation and movement
 *  - Continuous collision handling
 *  - Trash creation and removal
 *  - Universe collapse detection
 *
 * @param args Pointer to a net_thread_ctx structure.
 * @return NULL when thread terminates.
 */
void *physics_thread(void *args);

#endif /* THREADS_SERVER_H */
