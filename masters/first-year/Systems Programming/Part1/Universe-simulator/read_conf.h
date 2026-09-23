
#include <SDL2/SDL_ttf.h>
#include <libconfig.h>
#include "structs.h"


int open_conf_file(config_t *cfg, char *filename);

int read_configurations(config_t *cfg, int *window_size, int *n_planets, int *max_trash, int *initial_trash, int *ship_capacity, int *planet_radius, int *trash_radius, int *player_radius);

SDL_Color getColor(config_t *cfg, const char *color_name);
