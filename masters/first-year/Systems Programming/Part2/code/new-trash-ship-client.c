
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <zmq.h>
#include "common_files/read_conf.h"
#include "structs.h"
#include "Comunnication.h"
#include "display.h"
#include "read_conf.h"
#include "thread.h"
#include "trash_ship_only/thread.h"

#define FRAME_TIME_MS 33   // ~30 FPS
#define SEND_INTERVAL_MS 1000

pthread_mutex_t world_mutex;
Uint32 WORLD_UPDATE_EVENT;
const char *server_ip;
const char *port;
const char *sub_port;

int main(int argc, char **argv) {
    srand(time(NULL));

    char ip_connect[64] = "127.0.0.1";
  
    /* ---------- SDL INIT ---------- */
    SDL_Window   *window   = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font     *font     = NULL;

    config_t cfg;
    if (open_conf_file(&cfg, "config.conf"))
        return 1;

    SDL_Color bg0   = getColor(&cfg, "white");
    SDL_Color black = getColor(&cfg, "black");

    int window_size, n_planets_conf, max_trash, initial_trash;
    int ship_capacity, planet_radius, trash_radius, player_radius;

    if (!read_configurations(&cfg, &window_size, &n_planets_conf,
                             &max_trash, &initial_trash,
                             &ship_capacity, &planet_radius,
                             &trash_radius, &player_radius))
        return 1;


    if (!initialize_SDL_client(window_size, &window, &renderer, bg0))
        return 1;

    if (initialize_SDL_font(&font, 20, "./HackNerdFontMono-Regular.ttf"))
        return 1;


    //Create SDL event
    //
    WORLD_UPDATE_EVENT = SDL_RegisterEvents(1);
    if (WORLD_UPDATE_EVENT == (Uint32)-1) {
        fprintf(stderr, "Failed to register SDL event\n");
        exit(1);
    }


    /* -----------CONNECT to Server-----------*/

    if(!read_network_config(&cfg, &server_ip, &sub_port, &port))
        return 1;


    if (argc >= 2){
        snprintf(ip_connect, sizeof(ip_connect), "%s", argv[1]);
    }else{

        snprintf(ip_connect, sizeof(ip_connect), "%s", server_ip);
}

    void *context = zmq_ctx_new(); 
    void *fd_req = create_client_channel(ip_connect, context, port);
    void *fd_sub = create_sub_channel(ip_connect, context, sub_port);

    ship player;
    int connected = 0;

    // Try connecting to server
    for (int i = 0; i < 5; i++) {
        send_connection_message(fd_req);
        if (receive_response_connection(fd_req, &player)) {
            connected = 1;
            break;
        }
        usleep(100000);
    }

    if (!connected) {
        printf("ERROR: Cannot connect to server\n");
        return 1;
    }


    /* ---------- WORLD STATE ---------- */
    planet_structure *planets = NULL;
    trash_structure  *trash   = NULL;
    ship             *ships   = NULL;
    size_t n_planets = 0, n_trash = 0, n_ships = 0;

    pthread_mutex_init(&world_mutex, NULL);

    int running   = 1;
    int has_world = 0;

    pthread_t sub_thread;
    sub_thread_args args = {
        fd_sub,
        &planets, &trash, &ships,
        &n_planets, &n_trash, &n_ships,
        &running,
        &has_world,
        WORLD_UPDATE_EVENT
    };
    pthread_create(&sub_thread, NULL, sub_thread_func, &args);

    /* ---------- MAIN LOOP ---------- */
    SDL_Event event;
    int send;
    int user = 1;
Uint32 last_send_time = 0;
    while (running) {
        // ===== Event handling =====
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
            {
                running = 0;
                send_exit_message(fd_req, &player);
                receive_response(fd_req);
               
            }

            if (event.type == SDL_KEYDOWN) {
                direction_t dir;
                send = 1;

                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:  dir = LEFT;  break;
                    case SDLK_RIGHT: dir = RIGHT; break;
                    case SDLK_UP:    dir = UP;    break;
                    case SDLK_DOWN:  dir = DOWN;  break;
                    case SDLK_ESCAPE:
                        running = 0;
                        send = 2;
                        break;
                    default:
                        send = 0;
                }

                if (send == 1) {
                    send_movement_message(fd_req, &player, dir);
                    receive_response(fd_req);
                }
                if (send == 2){
                    send_exit_message(fd_req, &player);
                    receive_response(fd_req);
                }
            }
        
        // ===== Rendering =====
        if (event.type == WORLD_UPDATE_EVENT) {
            switch (event.user.code) {
                  case WORLD_EVENT_UPDATE:
                  pthread_mutex_lock(&world_mutex);
            planet_structure *cur_planets = planets;
            trash_structure  *cur_trash   = trash;
            ship             *cur_ships   = ships;
            size_t cur_np = n_planets;
            size_t cur_nt = n_trash;
            size_t cur_ns = n_ships;
            pthread_mutex_unlock(&world_mutex);


            if (cur_planets && cur_trash && cur_ships) {
                render_all(renderer,
                           cur_planets, cur_np, planet_radius,
                           cur_trash,   cur_nt, trash_radius,
                           cur_ships,   cur_ns,
                           font, bg0, black);
            }


            break;
            case WORLD_EVENT_SERVER_CLOSED:

                printf("Server closed connection\n");
                draw_server_close(renderer, font, window_size, bg0, user);
                running = 0;
                      
                break;

            case WORLD_EVENT_COLAPSED:
                printf("World colapsed\n");
                draw_world_colapse_close(renderer, font, window_size, bg0, user);
                running = 0;
                break;

            case WORLD_SERVER_DOWN:
                printf("Impossible to connect Server \n");
                draw_server_collapsed(renderer, font, window_size, bg0);
                running = 0;
  goto exit_server;
                break;
            }
        }
    Uint32 now = SDL_GetTicks();    
    if (now - last_send_time >= SEND_INTERVAL_MS && running == 1) {
        last_send_time = now;

        // ---- SEND PERIODIC MESSAGE ----
        send_active_message(fd_req, &player);
        receive_response(fd_req);
            }
        exit_server:
        SDL_Delay(1);
   
    }
    /*Uint32 now = SDL_GetTicks();    
    if (now - last_send_time >= SEND_INTERVAL_MS && running == 1) {
        last_send_time = now;

        // ---- SEND PERIODIC MESSAGE ----
    
        send_active_message(fd_req,& player);
        receive_response(fd_req);
        }*/
        }

    /* ---------- CLEANUP ---------- */
    running = 0;
    zmq_ctx_shutdown(context);
    zmq_close(fd_sub);          // unblock recv
    pthread_join(sub_thread, NULL);

    pthread_mutex_lock(&world_mutex);
    free_world(planets, n_planets, trash, n_trash, ships);
    pthread_mutex_unlock(&world_mutex);

    pthread_mutex_destroy(&world_mutex);

    zmq_close(fd_req);
    zmq_ctx_term(context);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

