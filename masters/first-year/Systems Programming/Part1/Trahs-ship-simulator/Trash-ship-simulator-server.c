#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "SDL2/SDL_pixels.h"
#include <time.h>
#include <SDL2/SDL_ttf.h>
#include <libconfig.h>
#include "display.h"
#include "Universe-data.h"
#include "structs.h"
#include "read_conf.h"
#include "Comunnication.h"
#include "Player.h"

Uint32 timer_callback(Uint32 interval, void *param)
{
    SDL_Event event;
    SDL_memset(&event, 0, sizeof(event));
    event.type = SDL_USEREVENT;
    SDL_PushEvent(&event);
    return interval; // repeat
}



int main(){
    srand(time(NULL));   // seed RNG once so i can have difernt rand values on options
    

    // Open .conf file
    config_t cfg;
    
    if(open_conf_file(&cfg, "config.conf")){
    return 1;
    }

    //Get colors from config file

    SDL_Color red   = getColor(&cfg, "red");
    SDL_Color blue  = getColor(&cfg, "blue");
    SDL_Color green = getColor(&cfg, "green");
    SDL_Color bg0 = getColor(&cfg, "white");
    SDL_Color black = getColor(&cfg, "black");

    // Get configuration for game 
    int window_size, n_planets, max_trash, initial_trash, ship_capacity, planet_radius, trash_radius, player_radius;

    if(!read_configurations(&cfg, &window_size, &n_planets, &max_trash, &initial_trash, &ship_capacity, &planet_radius, &trash_radius, &player_radius)){
        printf("ERRROR reading data for confuration\n");
        return 1;
    }
   
    //Open game window
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!initialize_SDL_simple(window_size, &window, &renderer, bg0)) {
        return 1;
    }
    //Open font
    TTF_Font *font = NULL;
    int font_size = 20;
    if (initialize_SDL_font(&font, font_size, "./HackNerdFontMono-Regular.ttf") != 0) {
        return 1;
    }

    //Planet create and draw 
    planet_structure *planets = create_planet(n_planets, window_size, 10, &red, &blue);  
    draw_planet(renderer,planets, n_planets, planet_radius, font);

    trash_structure *trash = create_trash_init(&green, 1, window_size, initial_trash, max_trash);
    draw_trash(renderer, trash, initial_trash, trash_radius);
    
    ship  *ships = ship_init(n_planets, window_size);
    
    // Show on screen
    
    SDL_RenderPresent(renderer);
    SDL_AddTimer(500, timer_callback, NULL);

    // Open server chanel
    void * fd = create_server_channel();
    
    direction_t d;
    char message_type[100];
    char c;
    respost rsp;
    int close = 0;
    int i, pass;
    int pos_x, pos_y;
    int step = 10;
    int margin = 5;
    
    while (!close)
    {
        SDL_Event event;
        SDL_PollEvent(&event);

        switch (event.type) {
            case SDL_QUIT:
                close = 1;
            break;
        
            default:
        read_message (fd, message_type, &c, &d, &pass);
        if(strcmp(message_type, "HELLO")==0){
            if( (i = next_ship_use(ships, n_planets)) >= 0 ){ 
                rsp = RES_OK;
                send_response_connection (fd, rsp, &ships[i]);
                }else{
                continue;
            }
            render_all(renderer,planets, n_planets, planet_radius,
                        trash, initial_trash, trash_radius,
                        ships, n_planets,font,bg0, black);
        }

        if(strcmp(message_type, "MOVE")==0){
             int idx = check_info(ships, n_planets, c, pass);
            if(idx != -1){
                pos_x = ships[idx].y;
                pos_y = ships[idx].x;
                int error = new_position(&pos_x, &pos_y, d, window_size, step);
                ships[idx].y = pos_x;
                ships[idx].x = pos_y;
                int test =  hit(&ships[idx], trash, initial_trash,  n_planets, planets, &blue, margin);
                        if (test == 3){
                            spiled_trash(&ships[idx], trash, initial_trash, window_size);

                        }else if( test == 2){
                            recicle_trash(&ships[idx], trash, initial_trash);
                        }

                render_all(renderer, planets, n_planets, planet_radius,
                   trash, initial_trash, trash_radius,ships, 
                   n_planets,font,bg0, black);
        
                if (error){
                    rsp = RES_ERROR;
                    send_response (fd, rsp);
                }else{
                    rsp = RES_OK;
                    send_response (fd, rsp);
                }
            }
        }
        break;
        }
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();



    return 1;

}


