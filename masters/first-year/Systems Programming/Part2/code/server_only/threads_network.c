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

#define WORLD_ACTIVE_INTERVAL_MS 200   // Intervalo de atualização do mundo (ms)

/* ============================================================
   NETWORK THREAD
   ============================================================
   Thread responsável por:
   - Receber mensagens dos clientes
   - Atualizar o estado do mundo (ships, trash, planets)
   - Responder aos clientes
   ============================================================ */

void *network_thread(void *arg)
{
    net_thread_ctx *ctx = arg;

    // Inicializa o estado de atividade das naves
    memset(ship_active, 0, n_planets * sizeof(int));

    char type[32];        // tipo de mensagem recebida
    char c;               // identificador do jogador
    direction_t d;        // direção de movimento
    int pass;             // password / token de autenticação

    // Loop principal da thread enquanto o universo estiver ativo
    while (!universe_collapsed) {

        // Lê mensagem do cliente
        read_message(ctx->fd, type, &c, &d, &pass);

        // Protege acesso ao estado global do mundo
        pthread_mutex_lock(&world_mutex);

        /* -------- HELLO --------
           Pedido de ligação inicial de um cliente
           Ativa uma ship disponível e devolve os seus dados
        */
        if (strcmp(type, "HELLO") == 0) {

            int i = next_ship_use(ships, n_planets);
            ship_active[i] = 1;

            if (i >= 0) {
                ships[i].use = 1;
                ships[i].x = window_size / 2;
                ships[i].y = window_size / 2;
                ships[i].trash = 0;

                printf("Ship ativada: idx=%d x=%f y=%f use=%d\n",
                       i, ships[i].x, ships[i].y, ships[i].use);

                send_response_connection(ctx->fd, RES_OK, &ships[i]);
            } else {
                send_response(ctx->fd, RES_ERROR);
            }
        }

        /* -------- MOVE --------
           Atualiza o movimento da ship
           Aplica thrust, verifica colisões e interações
        */
        else if (strcmp(type, "MOVE") == 0) {

            int idx = check_info(ships, n_planets, c, pass);

            if (idx >= 0) {

                // Aplica impulso à nave na direção recebida
                new_ship_thrust(&ships[idx], d);
                int err = 0;

                // Verifica colisões com lixo ou planetas
                int hit_res = hit(&ships[idx],
                                  trash, current_trash,
                                  n_planets, planets,
                                  ctx->planet_color,
                                  ctx->margin);

                /* -------- Colisão com lixo -------- */
                if (hit_res >= 0 && hit_res < current_trash) {
                    trash[hit_res] = trash[current_trash - 1];
                    was_colliding[hit_res] = was_colliding[current_trash - 1];
                    current_trash--;
                }

                /* -------- Planeta de reciclagem -------- */
                else if (hit_res == -2) {
                    recicle_trash(&ships[idx], trash, current_trash);
                    ships[idx].trash = 0;
                }

                /* -------- Planeta de despejo -------- */
                else if (hit_res == -3) {
                    spiled_trash(&ships[idx],
                                 trash,
                                 &current_trash,
                                 max_trash,
                                 ctx->window_size,
                                 &green,
                                 was_colliding);

                    ships[idx].trash = 0;
                }

                send_response(ctx->fd,
                              err ? RES_ERROR : RES_OK);
            } else {
                send_response(ctx->fd, RES_ERROR);
            }
        }

        /* -------- EXIT --------
           Cliente sai do jogo
           Liberta a ship associada
        */
        else if (strcmp(type, "EXIT") == 0) {

            int idx = check_info(ships, n_planets, c, pass);

            if (idx >= 0) {
                ships[idx].use = 0;
                send_response(ctx->fd, RES_OK);
            } else {
                send_response(ctx->fd, RES_ERROR);
            }
        }

        /* -------- ACTIVE --------
           Marca ship como ativa (heartbeat / keep-alive)
        */
        else if (strcmp(type, "ACTIVE") == 0) {

            int idx = check_info(ships, n_planets, c, pass);

            if (idx >= 0) {
                ship_active[idx] = 1;
                send_response(ctx->fd, RES_OK);
            } else {
                send_response(ctx->fd, RES_ERROR);
            }
        }

        // Liberta mutex do mundo
        pthread_mutex_unlock(&world_mutex);

        // Pequena pausa para não saturar CPU
        usleep(1000);
    }

    return NULL;
}
