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
#include "structs.h"
#include "Comunnication.h"
#include "Player.h"
#include "threads_server.h"

/* ============================================================
   PHYSICS THREAD
   ============================================================
   Thread responsável por:
   - Atualizar a física do universo em passos temporais fixos
   - Calcular gravidade, velocidade e posição
   - Gerir colisões contínuas
   - Criar e remover lixo
   - Detetar colapso do universo
   ============================================================ */

void *physics_thread(void *args)
{
    net_thread_ctx *ctx = args;

    const Uint32 STEP_MS = 10;     // intervalo de atualização da física
    Uint32 last_physics = SDL_GetTicks();

    // Loop principal da thread enquanto o universo estiver ativo
    while (!universe_collapsed) {

        Uint32 now = SDL_GetTicks();

        // Executa física apenas quando o passo temporal é atingido
        if (now - last_physics >= STEP_MS) {
            last_physics = now;

            // Protege acesso ao estado global do mundo
            pthread_mutex_lock(&world_mutex);

            /* -------- Atualização da física -------- */
            new_trash_acceleration(planets, n_planets, trash, current_trash);
            new_trash_velocity(trash, current_trash);
            new_trash_position(trash, current_trash);

            new_ship_acceleration(planets, n_planets, ships, n_planets);
            new_ship_velocity(ships, n_planets);
            new_ship_position(ships, n_planets);

            /* -------- Remoção de lixo apanhado -------- */
            for (int i = 0; i < current_trash; ) {
                if (trash[i].taken) {
                    // substitui pelo último lixo ativo
                    trash[i] = trash[current_trash - 1];
                    was_colliding[i] = was_colliding[current_trash - 1];
                    current_trash--;
                } else {
                    i++;
                }
            }

            /* -------- Criação dinâmica de lixo -------- */
            for (int i = 0; i < current_trash; i++) {

                int is_colliding = 0;

                // Verifica se o lixo está a colidir com algum planeta
                for (int p = 0; p < n_planets; p++) {
                    float dx = (float)planets[p].x - (float)trash[i].x;
                    float dy = (float)planets[p].y - (float)trash[i].y;
                    float dist2 = dx * dx + dy * dy;

                    if (dist2 <= (float)trash_radius) {
                        is_colliding = 1;
                        break;
                    }
                }

                // Cria novo lixo apenas na transição de colisão
                if (is_colliding &&
                    !was_colliding[i] &&
                    current_trash < max_trash &&
                    has_active_ship(ships, n_planets)) {

                    printf("Curr trash %d\n", current_trash);

                    trash[current_trash] =
                        create_trash(&green, 1, window_size);
                    was_colliding[current_trash] = 0;
                    current_trash++;
                }

                // Atualiza estado de colisão
                was_colliding[i] = is_colliding;
            }

            /* -------- Verificação contínua de colisões ship-planeta -------- */
            if (has_active_ship(ships, n_planets)) {

                for (int idx = 0; idx < n_planets; idx++) {

                    if (ships[idx].use == 1) {

                        int hit_res = hit(&ships[idx],
                                          trash, current_trash,
                                          n_planets, planets,
                                          ctx->planet_color,
                                          ctx->margin);

                        // Planeta de reciclagem
                        if (hit_res == -2) {
                            recicle_trash(&ships[idx],
                                          trash, current_trash);
                        }
                        // Planeta de despejo
                        else if (hit_res == -3) {
                            spiled_trash(&ships[idx],
                                         trash,
                                         &current_trash,
                                         max_trash,
                                         ctx->window_size,
                                         &green,
                                         was_colliding);
                        }
                    }
                }
            }

            /* -------- Colapso do universo -------- */
            if (current_trash >= max_trash) {
                printf("O universo colapsou! Lixo = %d\n", current_trash);
                universe_collapsed = 1;
            }

            pthread_mutex_unlock(&world_mutex);
        }

        // Pequena pausa para evitar consumo excessivo de CPU
        SDL_Delay(1);
    }

    return NULL;
}
