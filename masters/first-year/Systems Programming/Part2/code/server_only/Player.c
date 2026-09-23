#include <stdio.h>
#include "structs.h"

extern int ship_capacity;

/* ============================================================
   SHIP IDENTIFICATION & VALIDATION
   ============================================================ */

/*
 * Verifica se a ship identificada na mensagem existe e está ativa.
 *
 * Compara o identificador (char) e a password recebidos com os dados
 * das ships guardadas. Se houver correspondência, devolve o índice.
 *
 * Retorna:
 *   índice da ship em caso de sucesso
 *   -1 se não houver correspondência válida
 */
int check_info(ship *ships, int n_ships, char c, int pass)
{
    for (int i = 0; i < n_ships; i++) {

        // ship name é um único carácter + '\0'
        if (ships[i].name[0] == c &&
            ships[i].pass == pass &&
            ships[i].use == 1) {

            return i;   // ship válida encontrada
        }
    }

    return -1; // nenhuma ship válida
}

/* ============================================================
   SHIP MOVEMENT (LEGACY / STEP-BASED)
   ============================================================ */

/*
 * Calcula nova posição da ship com movimento discreto (step).
 *
 * Implementa comportamento wrap-around: quando a ship sai do mapa,
 * reaparece no lado oposto.
 *
 * Retorna:
 *   0 - movimento válido
 *   1 - direção inválida
 */
int new_position(int* x, int *y, direction_t direction,
                 int WINDOW_SIZE, int step)
{
    int hit_wall = 0;

    switch (direction)
    {
        case UP:
            *x -= step;
            if (*x < 0) {
                *x = WINDOW_SIZE + *x;   // top -> bottom
            }
            break;

        case DOWN:
            *x += step;
            if (*x >= WINDOW_SIZE) {
                *x = WINDOW_SIZE - *x;   // bottom -> top
            }
            break;

        case LEFT:
            *y -= step;
            if (*y < 0) {
                *y = WINDOW_SIZE + *y;   // left -> right
            }
            break;

        case RIGHT:
            *y += step;
            if (*y >= WINDOW_SIZE) {
                *y = WINDOW_SIZE - *y;   // right -> left
            }
            break;

        default:
            hit_wall = 1;   // direção inválida
            break;
    }

    return hit_wall;
}

/* ============================================================
   COLOR UTILITIES
   ============================================================ */

/*
 * Compara duas cores SDL_Color.
 *
 * Retorna 1 se forem iguais, 0 caso contrário.
 */
int same_color(SDL_Color *a, SDL_Color *b) {
    return (a->r == b->r &&
            a->g == b->g &&
            a->b == b->b &&
            a->a == b->a);
}

/* ============================================================
   COLLISION DETECTION
   ============================================================ */

/*
 * Verifica colisões da ship com lixo ou planetas.
 *
 * Retornos possíveis:
 *   >= 0  : índice do lixo apanhado
 *   -1    : nenhuma colisão relevante
 *   -2    : colisão com planeta de reciclagem
 *   -3    : colisão com planeta de despejo
 */
int hit(ship *ships,
        trash_structure *trash, int curr_trash,
        int n_planets, planet_structure *planets,
        SDL_Color *recicling, int margin)
{
    /* -------- Verificação de colisão com lixo -------- */
    for (int i = 0; i < curr_trash; i++) {

        float dx = ships->x - trash[i].x;
        float dy = ships->y - trash[i].y;

        float hit_dist = (player_radius * 2.0f) + trash_radius;

        if ((dx * dx + dy * dy) <= (hit_dist * hit_dist) &&
            trash[i].taken == 0)
        {
            // Ship cheia: não apanha mais lixo
            if (ships->trash >= ship_capacity) {
                printf("SHIP FULL\n");
                return -1;
            }

            ships->trash++;
            trash[i].taken = 1;
            strcpy(trash[i].ship_name, ships->name);

            printf("TRASH COLLECTED\n");
            return i;   // devolve índice do lixo apanhado
        }
    }

    /* -------- Verificação de colisão com planetas -------- */
    for (int i = 0; i < n_planets; i++) {

        float dx = ships->x - planets[i].x;
        float dy = ships->y - planets[i].y;

        float hit_dist = player_radius + margin;

        if ((dx * dx + dy * dy) <= hit_dist )
        {
            // Planeta de reciclagem
            if (same_color(planets[i].color, recicling)) {
                printf("RECICLING PLANET\n");

                planets[i].trash += ships->trash;
                ships->trash = 0;

                return -2;
            }
            // Planeta de despejo
            else {
                return -3;
            }
        }
    }

    return -1; // nenhuma colisão
}
