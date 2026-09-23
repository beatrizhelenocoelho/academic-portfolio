#include "structs.h"
#include "threads_server.h"
#include <SDL2/SDL_pixels.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

/* ============================================================
   PLANET CREATION
   ============================================================ */

/*
 * Cria o array de planetas com posições aleatórias.
 *
 * Todos os planetas começam com a mesma cor base e massa.
 * Um planeta é escolhido aleatoriamente como planeta de reciclagem,
 * recebendo uma cor diferente.
 *
 * Retorna:
 *   Ponteiro para o array de planetas criado
 *   NULL em caso de falha de alocação
 */
planet_structure *create_planet(int n_planets, int window_size,
                                 int mass, SDL_Color *main,
                                 SDL_Color *recicling)
{
    planet_structure *planets =
        malloc(n_planets * sizeof(planet_structure));

    if (!planets){
        return NULL;
    }

    for (int i = 0; i < n_planets; i++){
        planets[i].x = rand() % window_size;
        if(planets[i].x < 20)
            planets[i].x = 20;
        planets[i].y = rand() % window_size;
        if(planets[i].y < 20)
            planets[i].y = 20;

        planets[i].color = main;
        planets[i].mass  = mass;
        planets[i].trash = 0;

        // Nome do planeta: uma letra (A–Z)
        planets[i].name[0] = 'A' + (i % 26);
        planets[i].name[1] = '\0';
    }

    // Seleciona aleatoriamente o planeta de reciclagem
    int recicle = rand() % n_planets;
    planets[recicle].color = recicling;

    return planets;
}

/* ============================================================
   TRASH CREATION
   ============================================================ */

/*
 * Cria um objeto de lixo individual com posição aleatória.
 *
 * O lixo começa não apanhado (taken = 0).
 */
trash_structure create_trash(SDL_Color *color, int mass, int window_size)
{
    trash_structure trash;

    trash.x = rand() % window_size;
    trash.y = rand() % window_size;
    trash.mass = mass;
    trash.color = color;
    trash.taken = 0;

    return trash;
}

/*
 * Aloca o array de lixo e inicializa o lixo inicial do universo.
 *
 * Apenas os primeiros num_trash_init elementos são inicializados;
 * o array pode conter espaço para crescer até num_trash.
 */
trash_structure *create_trash_init(SDL_Color *color, int mass,
                                   int window_size,
                                   int num_trash_init,
                                   int num_trash)
{
    trash_structure *trash =
        malloc(num_trash * sizeof(trash_structure));

    for (int i = 0; i < num_trash_init; i++){
        trash[i] = create_trash(color, mass, window_size);
    }

    return trash;
}

/* ============================================================
   SHIP CREATION AND MANAGEMENT
   ============================================================ */

/*
 * Cria e inicializa as ships do jogo.
 *
 * As ships começam inativas (use = 0), com posições aleatórias
 * e passwords geradas aleatoriamente.
 */
ship *ship_init(int n_ships, int window_size)
{
    ship *ships = malloc(n_ships * sizeof(ship));

    for (int i = 0; i < n_ships; i++){
        ships[i].x = window_size / 2;
        if(ships[i].x < 30)
            ships[i].x = 30;
        ships[i].y = window_size / 2;
        if(ships[i].y < 30)
            ships[i].y = 30;

        ships[i].name[0] = 'A' + (i % 26);
        ships[i].name[1] = '\0';

        ships[i].trash = 0;
        ships[i].use   = 0;
        ships[i].pass  = rand() % 256;
    }

    return ships;
}

/*
 * Procura a primeira ship livre e marca-a como ativa.
 *
 * Retorna:
 *   Índice da ship ativada
 *   -1 se não houver ships disponíveis
 */
int next_ship_use(ship *ships, int n_ships)
{
    for (int i = 0; i < n_ships; i++){
        if (ships[i].use == 0){
            ships[i].use = 1;
            return i;
        }
    }

    return -1;
}

/* ============================================================
   TRASH INTERACTION
   ============================================================ */

/*
 * Espalha o lixo transportado por uma ship pelo universo.
 *
 * O lixo é largado em posições aleatórias até esgotar a carga
 * da ship ou atingir o limite máximo de lixo permitido.
 */
void spiled_trash(ship *ship,
                  trash_structure *trash,
                  int *current_trash,
                  int max_trash,
                  int window_size,
                  SDL_Color *trash_color,
                  int *was_colliding)
{
    int amount = ship->trash;
    if (amount <= 0) return;

    for (int k = 0; k < amount; k++) {

        if (*current_trash >= max_trash)
            break;

        trash[*current_trash] =
            create_trash(trash_color, 1, window_size);

        was_colliding[*current_trash] = 0;
        (*current_trash)++;
    }

    ship->trash = 0;  // ship fica vazia após despejo
}

/*
 * Marca o lixo transportado por uma ship como reciclado.
 *
 * Identifica o lixo pelo nome da ship que o recolheu.
 */
void recicle_trash(ship *ships,
                   trash_structure *trash,
                   int n_trash)
{
    for (int i = 0; i < n_trash; i++){
        if (strcmp(trash[i].ship_name, ships->name) == 0 &&
            trash[i].taken == 1) {

            trash[i].taken = 2;
        }
    }
}

/* ============================================================
   SHIP ACTIVITY CHECK
   ============================================================ */

/*
 * Verifica se existe pelo menos uma ship ativa no universo.
 *
 * Retorna:
 *   1 se existir uma ship ativa
 *   0 caso contrário
 */
int has_active_ship(ship *ships, int n_ships)
{
    for (int i = 0; i < n_ships; i++) {
        if (ships[i].use == 1) {
            return 1;
        }
    }
    return 0;
}
