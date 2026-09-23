
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "SDL2/SDL_pixels.h"
#include <SDL2/SDL_ttf.h>
#include <libconfig.h>



//===============================///////
//TO get colors from config file
//===============================////////////
//
SDL_Color getColor(config_t *cfg, const char *color_name)
{
    SDL_Color color = {255, 255, 255, 255}; // default in case error happens
    config_setting_t *colors = config_lookup(cfg, "colors"); // top-level color structure
    if (!colors){
        return color;
    }
    config_setting_t *c = config_setting_get_member(colors, color_name);
    if (c) {
        int r, g, b, a;

        if (config_setting_lookup_int(c, "r", &r) &&
            config_setting_lookup_int(c, "g", &g) &&
            config_setting_lookup_int(c, "b", &b) &&
            config_setting_lookup_int(c, "a", &a)) {
            color.r = (Uint8)r;
            color.g = (Uint8)g;
            color.b = (Uint8)b;
            color.a = (Uint8)a;
        }
    }
    return color;
}


//============================///////
//Read configuration from config file////
//============================////////
int read_configurations(config_t *cfg, int *window_size, int *n_planets, int *max_trash, int *initial_trash, int *ship_capacity, int *planet_radius, int *trash_radius, int *player_radius)
{

    config_setting_t *root = config_root_setting(cfg);  //Opening root settings 
    if (!root) {
        printf("ERROR: no root setting\n");
        return 0;
    }

    if(config_setting_lookup_int(root, "planet_number", n_planets) &&
    config_setting_lookup_int(root, "max_trash", max_trash) &&
    config_setting_lookup_int(root, "initial_trash", initial_trash) &&
    config_setting_lookup_int(root, "trash_capacity", ship_capacity) &&
    config_setting_lookup_int(root, "window_size", window_size) &&
    config_setting_lookup_int(root, "planet_radius", planet_radius) &&
    config_setting_lookup_int(root, "trash_radius", trash_radius) &&
    config_setting_lookup_int(root, "player_radius", planet_radius)){
        return 1;
    }
    printf("ERROR read_config\n");

    return 0;

}

//==========================//////////
//  Read config file
//==========================//////////
int open_conf_file(config_t *cfg, char *filename)
{
    config_init(cfg);

    if (!config_read_file(cfg, filename)) {
        fprintf(stderr, "%s:%d - %s\n",
                config_error_file(cfg),
                config_error_line(cfg),
                config_error_text(cfg));
        config_destroy(cfg);
        return 1;
    }

    return 0;

}
//============================///////
//Read network configuration from config file////
//============================////////
int read_network_config(config_t *cfg,const char **ip_server,
                        const char **pub_port,const char **port)
{
    config_setting_t *root = config_root_setting(cfg);
    if (!root) {
        printf("ERROR: no root setting\n");
        return 0;
    }
if(ip_server != NULL){
    if (config_setting_lookup_string(root, "ip_server", ip_server) &&
        config_setting_lookup_string(root, "pub_port", pub_port) &&
        config_setting_lookup_string(root, "port", port)) {
        return 1;
    }
    }else{
    if (config_setting_lookup_string(root, "pub_port", pub_port) &&
        config_setting_lookup_string(root, "port", port)) {
        return 1;
        }  
    }

    printf("ERROR: read_network_config\n");
    return 0;
}

