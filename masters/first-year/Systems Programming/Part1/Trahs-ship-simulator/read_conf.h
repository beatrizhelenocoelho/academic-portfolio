#ifndef READ_CONF_H
#define READ_CONF_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <libconfig.h>
#include "structs.h"

// ============================================================================
// Open configuration file
// ----------------------------------------------------------------------------
// Loads the configuration file into a config_t structure.
//
// Parameters:
//   cfg      - pointer to a config_t object (must be initialized)
//   filename - path to the configuration file
//
// Returns:
//   1 on success, 0 on failure.
// ============================================================================
int open_conf_file(config_t *cfg, char *filename);

// ============================================================================
// Read all simulation parameters from configuration
// ----------------------------------------------------------------------------
// Reads window size, planet/trash counts, ship capacity, radii, etc.
//
// Parameters:
//   cfg            - pointer to loaded config_t object
//   window_size    - pointer to store window size
//   n_planets      - pointer to store number of planets
//   max_trash      - pointer to store maximum trash count
//   initial_trash  - pointer to store initial trash count
//   ship_capacity  - pointer to store ship carrying capacity
//   planet_radius  - pointer to store planet drawing radius
//   trash_radius   - pointer to store trash drawing radius
//   player_radius  - pointer to store player/ship drawing radius
//
// Returns:
//   1 on success, 0 on failure.
// ============================================================================
int read_configurations(config_t *cfg, int *window_size, int *n_planets,
                        int *max_trash, int *initial_trash, int *ship_capacity,
                        int *planet_radius, int *trash_radius, int *player_radius);

// ============================================================================
// Get SDL_Color from configuration
// ----------------------------------------------------------------------------
// Reads a color (r,g,b,a) from the config file by name.
//
// Parameters:
//   cfg        - pointer to loaded config_t object
//   color_name - name of the color in the config file
//
// Returns:
//   SDL_Color struct with color values (0-255).
// ============================================================================
SDL_Color getColor(config_t *cfg, const char *color_name);

#endif

