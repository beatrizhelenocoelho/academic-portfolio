#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_ttf.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "Comunnication.h"
#include "display.h"
#include "read_conf.h"
#include "thread.h"

/* ============================================================
   WORLD MEMORY MANAGEMENT
   ============================================================ */

/*
 * Liberta toda a memória associada ao estado do mundo.
 *
 * Liberta:
 *  - cores alocadas dinamicamente
 *  - arrays de planetas, lixo e ships
 *
 * Deve ser usada apenas quando o mundo deixa de ser necessário.
 */
void free_world(planet_structure *planets, size_t n_planets,
                trash_structure *trash, size_t n_trash,
                ship *ships)
{
    if (planets) {
        for (size_t i = 0; i < n_planets; i++) {
            free(planets[i].color);
        }
        free(planets);
    }

    if (trash) {
        for (size_t i = 0; i < n_trash; i++) {
            free(trash[i].color);
        }
        free(trash);
    }

    free(ships);
}

/* ============================================================
   SUBSCRIBER THREAD
   ============================================================
   Thread responsável por:
   - Receber estados do mundo via ZMQ (SUB)
   - Atualizar o estado local do cliente
   - Comunicar alterações ao loop SDL através de eventos
   ============================================================ */

void *sub_thread_func(void *arg)
{
    sub_thread_args *args = (sub_thread_args *)arg;

    int flag = 0;
    int return_read;

    SDL_Event ev;
    SDL_zero(ev);
    ev.type = args->world_update;

    // Loop principal da thread enquanto o cliente estiver ativo
    while (*args->running) {

        planet_structure *p = NULL;
        trash_structure  *t = NULL;
        ship             *s = NULL;
        size_t n_p = 0, n_t = 0, n_s = 0;

        // Recebe novo estado do mundo do servidor
        return_read =
            receive_world_state(args->fd_sub,
                                &p, &n_p,
                                &t, &n_t,
                                &s, &n_s,
                                &flag);

        /* -------- Estado recebido com sucesso -------- */
        if (return_read == 0) {

            if (p && t && s) {
                pthread_mutex_lock(&world_mutex);

                /*
                 * Nota:
                 * Libertar o mundo anterior pode causar pequenos problemas
                 * temporários no SDL. Mantido comentado para evitar bugs.
                 */
                // free_world(*args->planets, *args->n_planets,
                //            *args->trash,   *args->n_trash,
                //            *args->ships);

                /* Atualização atómica do mundo */
                *args->planets   = p;
                *args->trash     = t;
                *args->ships     = s;
                *args->n_planets = n_p;
                *args->n_trash   = n_t;
                *args->n_ships   = n_s;
                *args->has_world = 1;

                pthread_mutex_unlock(&world_mutex);

                ev.user.code = WORLD_EVENT_UPDATE;
                SDL_PushEvent(&ev);
            }
        }

        /* -------- Força evento de atualização -------- */
        SDL_Event ev;
        SDL_zero(ev);
        ev.type = args->world_update;
        SDL_PushEvent(&ev);   // THREAD-SAFE

        /* -------- Servidor encerrou o mundo -------- */
        if (return_read == 5){
            *args->has_world = 5;
            ev.user.code = WORLD_EVENT_SERVER_CLOSED;
            SDL_PushEvent(&ev);
        }

        /* -------- Universo colapsou -------- */
        if (return_read == 6){
            *args->has_world = 6;
            ev.user.code = WORLD_EVENT_COLAPSED;
            SDL_PushEvent(&ev);
        }

        /* -------- Servidor caiu -------- */
        if (return_read == -1){
            *args->has_world = -1;
            ev.user.code = WORLD_SERVER_DOWN;
            SDL_PushEvent(&ev);
        }

        // Pequena pausa para evitar consumo excessivo de CPU
        usleep(5000);  // 5 ms
    }

    return NULL;
}
