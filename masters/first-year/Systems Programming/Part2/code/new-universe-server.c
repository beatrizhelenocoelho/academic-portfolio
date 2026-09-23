#include <SDL2/SDL.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include <zmq.h>
#include <libconfig.h>
#include "read_conf.h"
#include "gravitation.h"
#include "display.h"
#include "Universe-data.h"
#include "server_only/threads_server.h"
#include "structs.h"
#include "read_conf.h"
#include "Comunnication.h"
#include "Player.h"


#define WORLD_ACTIVE_INTERVAL_MS 2000   // 1 second


/* ============================================================
   GLOBAL SHARED STATE
   ============================================================ */

planet_structure *planets = NULL;
trash_structure  *trash   = NULL;
ship *ships = NULL;
int *ship_active = NULL;

int n_planets      = 0;
int initial_trash  = 0;
int max_trash      = 0;
int trash_radius   = 0;
int window_size    = 0;
int player_radius = 0;
int planet_radius = 0;
int ship_capacity = 0;

int current_trash = 0;
int quit = 0;
int universe_collapsed = 0;

int *was_colliding = NULL;

pthread_mutex_t world_mutex = PTHREAD_MUTEX_INITIALIZER;

const char *port;
const char *port_sub;

/* Colors */
SDL_Color red;
SDL_Color blue;
SDL_Color green;
SDL_Color bg0;
SDL_Color black;




/* ============================================================
   MAIN
   ============================================================ */

int main(void)
{
    srand(time(NULL));
    /* ---------- CONFIG ---------- */
    config_t cfg;
    if (open_conf_file(&cfg, "config.conf"))
        return 1;

    red   = getColor(&cfg, "red");
    blue  = getColor(&cfg, "blue");
    green = getColor(&cfg, "green");
    bg0   = getColor(&cfg, "white");
    black = getColor(&cfg, "black");


    if (!read_configurations(&cfg, &window_size, &n_planets,
                             &max_trash, &initial_trash,
                             &ship_capacity,
                             &planet_radius,
                             &trash_radius,
                             &player_radius))
        return 1;
    if (!read_network_config(&cfg, NULL, &port_sub, &port))
        return 1;

    /* ---------- SDL ---------- */
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!initialize_SDL_server(window_size, &window, &renderer, bg0))
        return 1;

    set_universe_size(window_size);     
    
    TTF_Font *font = NULL;
    if (initialize_SDL_font(&font, 20,
        "./HackNerdFontMono-Regular.ttf") != 0)
        return 1;

    /* ---------- WORLD INIT ---------- */
    planets = create_planet(n_planets, window_size, 10, &red, &blue);
    trash   = create_trash_init(&green, 1, window_size,
                                initial_trash, max_trash);
    ships   = ship_init(n_planets, window_size);

    if (planets == NULL) {
        printf("ERROR creating planets\n");
        return 1;
    }   
    // descobrir qual é o planeta recicling planet 
    int recycling_idx = 0;
    for (int i = 0; i < n_planets; i++) {
        if (planets[i].color == &blue) {
            recycling_idx = i;
            break;
        }
    }
    
    if (trash == NULL) {
        printf("ERROR creating trash\n");
        free(planets);
        return 1;
    }

ship_active = calloc(n_planets, sizeof(int));
if (!ship_active) {
    perror("calloc ship_active");
    exit(1);
}
memset(ship_active, 0, n_planets * sizeof(int));



    // este array serve para saber, para cada bola de lixo,
    // se no frame anterior ela já estava em colisão com algum planeta.
    // assim, só crio lixo novo quando a colisão acontece outra vez
    // e não enquanto a bola está colada ao planeta.
    was_colliding = calloc(max_trash, sizeof(int));
    if (was_colliding == NULL) {
        printf("ERROR allocating was_colliding\n");
        free(planets);
        free(trash);
        return 1;
    }




    /* ---------- ZMQ INIT ---------- */
    void *context = zmq_ctx_new();
    void *fd  = create_server_channel(context, port);
    void *pub = create_pub_channel(context, port_sub);

    int running = 1;

    net_thread_ctx ctx = {
        .fd = fd,
        .pub = pub,
        .running = &running,
        .window_size = window_size,
        .step = 10,
        .margin = 50,
        .planet_color = &blue
    };
    

    //ZMQ Thread
    pthread_t net_thread;
    pthread_create(&net_thread, NULL, network_thread, &ctx);
    int seq = 1;

    //Physics Thread
    current_trash = initial_trash;  
    //quit = 0;                       // controlo do loop principal
    universe_collapsed = 0;         // fica a 1 quando o universo enche de lixo e para saber que game over!!!
    pthread_t physics_tid;
    pthread_create(&physics_tid, NULL, physics_thread, &ctx);
    

    //const Uint32 STEP_MS   = 10;   // física (100 Hz)
    const Uint32 RENDER_MS = 33;   // render (~30 Hz)
    const Uint32 SEND_WORLD_UPD_MS = 20;  //World update (20Hz)


    Uint32 last_physics = SDL_GetTicks();
    Uint32 last_render  = SDL_GetTicks();

    Uint32 last_trash_gen = SDL_GetTicks(); // tentar meter a criar lixo a cada 10s
    const Uint32 TRASH_GEN_MS = 10000; // 10s
    
    Uint32 last_recycle_change = SDL_GetTicks();
    const Uint32 RECYCLE_CHANGE_MS = 30000; // 30s
    SDL_Event event;
Uint32 last_publish_time = 0;
    /* ---------- MAIN LOOP (SDL ONLY) ---------- */
    while (!universe_collapsed) {
     //   SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT){
                quit = 1;
                universe_collapsed = 1;
                send_world_state_pub(pub, planets, n_planets, trash,
                                current_trash, ships, n_planets, -1);
            } else if (event.type == SDL_KEYDOWN &&
                       event.key.keysym.sym == SDLK_ESCAPE) {
                quit = 1;
                universe_collapsed = 1;
                send_world_state_pub(pub, planets, n_planets, trash,
                                current_trash, ships, n_planets, -1);
 
            }


        }
        
        Uint32 now = SDL_GetTicks();
        
        // 10s: gerar lixo novo
        if (has_active_ship(ships, n_planets) && (now - last_trash_gen) >= TRASH_GEN_MS) {
            last_trash_gen = now;
            pthread_mutex_lock(&world_mutex);
            if (current_trash < max_trash) {
                trash[current_trash] = create_trash(&green, 1, window_size);
                was_colliding[current_trash] = 0;
                current_trash++;
            }
            pthread_mutex_unlock(&world_mutex);
        }

        // 30s: mudar recycle planet
        if ((now - last_recycle_change) >= RECYCLE_CHANGE_MS) {
            last_recycle_change = now;
            pthread_mutex_lock(&world_mutex);
            // tirar azul ao planeta antigo
            planets[recycling_idx].color = &red;

            // escolher novo planeta diferente
            int new_idx = recycling_idx;
            while (new_idx == recycling_idx && n_planets > 1) {
                new_idx = rand() % n_planets;
            }

            recycling_idx = new_idx;
            planets[recycling_idx].color = &blue;
            pthread_mutex_unlock(&world_mutex);
            printf("[RECYCLE] planeta %d agora é recycle\n", recycling_idx);
        }
        // Send world update  50 Hz
        if(now - last_physics >= SEND_WORLD_UPD_MS){
        last_physics = now;
            pthread_mutex_lock(&world_mutex);
                    send_world_state_pub(pub,
                             planets, n_planets,
                             trash, current_trash,
                             ships, n_planets,
                             seq++);
            pthread_mutex_unlock(&world_mutex);


        }
        // -------- RENDER (30 Hz) --------
        if (now - last_render >= RENDER_MS) {
            last_render = now;
        pthread_mutex_lock(&world_mutex);
        render_all(renderer,
                   planets, n_planets, planet_radius,
                   trash, current_trash, trash_radius,
                   ships, n_planets,
                   font, bg0, black);
        pthread_mutex_unlock(&world_mutex);
        }
    if (now - last_publish_time >= WORLD_ACTIVE_INTERVAL_MS) {
        last_publish_time = now;
   pthread_mutex_lock(&world_mutex);
            for(int i = 0; i <n_planets; i++){
                if(ships[i].use == 1 && ship_active[i] == 0){

                    ships[i].use = 0;
                }
                ship_active[i] = 0;
            }

  pthread_mutex_unlock(&world_mutex);
        }


        SDL_Delay(1);
    }







    pthread_join(physics_tid, NULL);

    // -------------------------
    // 7) Ecrã final quando o universo fica cheio de lixo OR quit game ----- GAME OVER!!!!!!!!!!!!!!!!!!!-------
    // -------------------------
    //
    if (quit){ 
        draw_server_close(renderer, font, window_size, bg0, 0);

    }else if (universe_collapsed) {
       
        send_world_state_pub(pub, planets, n_planets, trash,
                      current_trash, ships, n_planets, 0);
        draw_world_colapse_close(renderer, font, window_size, bg0, 0);

    }
/* ---------- SHUTDOWN ---------- */

    // Tell the network thread to stop

    // Unblock the network thread if it's waiting on zmq_recv
    //    This wakes up any blocking recv calls
    zmq_ctx_shutdown(context);

    //Wait for the thread to exit
    pthread_join(net_thread, NULL);

    //Now safe to close sockets
    zmq_close(fd);
    zmq_close(pub);

    //Terminate the context
    zmq_ctx_term(context);


    // SDL cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();


    return 0;
}

