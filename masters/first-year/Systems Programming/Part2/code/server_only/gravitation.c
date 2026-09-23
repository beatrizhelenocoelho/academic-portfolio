#include <math.h>
#include "structs.h"

/* ============================================================
   GRAVITATIONAL PHYSICS & MOVEMENT
   ============================================================
   Implementa a física do universo:
   - Vetores de força
   - Gravitação entre planetas, lixo e ships
   - Atualização de aceleração, velocidade e posição
   ============================================================ */

static int UNIVERSE_SIZE = 800;

/* ------------------------------------------------------------
   Define o tamanho do universo (bordas com efeito wrap)
   ------------------------------------------------------------ */
void set_universe_size(int size) {
    UNIVERSE_SIZE = size;
}

/* ------------------------------------------------------------
   Cria um vetor a partir de componentes cartesianas
   ------------------------------------------------------------
   dx, dy  - componentes do vetor
   retorna - vetor em forma polar (amplitude + ângulo)
   ------------------------------------------------------------ */
static vector make_vector(float dx, float dy) {
    vector v;
    v.amplitude = sqrtf(dx * dx + dy * dy);
    v.angle = atan2f(dy, dx);
    return v;
}

/* ------------------------------------------------------------
   Soma dois vetores em forma polar
   ------------------------------------------------------------
   Converte para cartesiano, soma e reconverte para polar
   ------------------------------------------------------------ */
static vector add_vectors(vector a, vector b) {
    float ax = a.amplitude * cosf(a.angle);
    float ay = a.amplitude * sinf(a.angle);

    float bx = b.amplitude * cosf(b.angle);
    float by = b.amplitude * sinf(b.angle);

    float rx = ax + bx;
    float ry = ay + by;

    vector r;
    r.amplitude = sqrtf(rx * rx + ry * ry);
    r.angle = atan2f(ry, rx);
    return r;
}

/* ------------------------------------------------------------
   Corrige coordenadas inteiras (wrap do universo)
   ------------------------------------------------------------
   Se a posição sai fora do limite, reaparece do lado oposto
   ------------------------------------------------------------ */
static void correct_position(int *coord) {
    if (*coord < 0) {
        *coord += UNIVERSE_SIZE;
    } else if (*coord >= UNIVERSE_SIZE) {
        *coord -= UNIVERSE_SIZE;
    }
}

/* ------------------------------------------------------------
   Corrige coordenadas em float (wrap do universo)
   ------------------------------------------------------------ */
static void correct_position_f(float *coord)
{
    if (*coord < 0)
        *coord += UNIVERSE_SIZE;
    else if (*coord >= UNIVERSE_SIZE)
        *coord -= UNIVERSE_SIZE;
}

/* ============================================================
   TRASH PHYSICS
   ============================================================ */

/* ------------------------------------------------------------
   Calcula aceleração do lixo devido à gravidade dos planetas
   ------------------------------------------------------------
   Aplica força gravitacional de todos os planetas a cada lixo
   ------------------------------------------------------------ */
void new_trash_acceleration(planet_structure planets[], int total_planets,
                            trash_structure trash[], int total_trash)
{
    vector total_vector_force;

    for (int n_trash = 0; n_trash < total_trash; n_trash++){
        total_vector_force.amplitude = 0;
        total_vector_force.angle = 0;

        for (int n_planet = 0; n_planet < total_planets; n_planet++){
            float force_vector_x =
                (float)planets[n_planet].x - (float)trash[n_trash].x;
            float force_vector_y =
                (float)planets[n_planet].y - (float)trash[n_trash].y;

            vector local_vector_force =
                make_vector(force_vector_x, force_vector_y);

            // Evita divisão por zero quando coincide com o planeta
            if (local_vector_force.amplitude == 0) {
                continue;
            }

            // Força gravitacional (G ajustado para tornar o movimento visível)
            local_vector_force.amplitude =
                50 * (planets[n_planet].mass * trash[n_trash].mass) /
                powf(local_vector_force.amplitude, 2);

            total_vector_force =
                add_vectors(local_vector_force, total_vector_force);
        }

        trash[n_trash].acceleration = total_vector_force;
    }
}

/* ============================================================
   SHIP PHYSICS
   ============================================================ */

/* ------------------------------------------------------------
   Calcula aceleração das ships devido aos planetas
   ------------------------------------------------------------
   Massa da ship é considerada 1 (conforme enunciado)
   ------------------------------------------------------------ */
void new_ship_acceleration(planet_structure planets[], int total_planets,
                           ship ships[], int total_ships)
{
    vector total_vector_force;

    for (int s = 0; s < total_ships; s++){
        total_vector_force.amplitude = 0;
        total_vector_force.angle = 0;

        for (int p = 0; p < total_planets; p++){
            float force_vector_x =
                (float)planets[p].x - ships[s].x;
            float force_vector_y =
                (float)planets[p].y - ships[s].y;

            vector local_vector_force =
                make_vector(force_vector_x, force_vector_y);

            if (local_vector_force.amplitude == 0) {
                continue;
            }

            local_vector_force.amplitude =
                2.0f * planets[p].mass /
                powf(local_vector_force.amplitude, 2);

            total_vector_force =
                add_vectors(local_vector_force, total_vector_force);
        }

        ships[s].acceleration = total_vector_force;
    }
}

/* ------------------------------------------------------------
   Atualiza velocidade do lixo
   ------------------------------------------------------------
   Aplica atrito e soma aceleração
   ------------------------------------------------------------ */
void new_trash_velocity(trash_structure trash[], int total_trash){
    for (int n_trash = 0; n_trash < total_trash; n_trash++){
        trash[n_trash].velocity.amplitude *= 0.99f; // atrito
        trash[n_trash].velocity =
            add_vectors(trash[n_trash].velocity,
                        trash[n_trash].acceleration);
    }
}

/* ------------------------------------------------------------
   Atualiza velocidade das ships
   ------------------------------------------------------------ */
void new_ship_velocity(ship ships[], int total_ships)
{
    for (int s = 0; s < total_ships; s++){
        ships[s].velocity.amplitude *= 0.99f; // atrito
        ships[s].velocity =
            add_vectors(ships[s].velocity,
                        ships[s].acceleration);
    }
}

/* ------------------------------------------------------------
   Atualiza posição do lixo
   ------------------------------------------------------------ */
void new_trash_position(trash_structure trash[], int total_trash){
    for (int n_trash = 0; n_trash < total_trash; n_trash++){
        trash[n_trash].x +=
            (int)(trash[n_trash].velocity.amplitude *
                  cosf(trash[n_trash].velocity.angle));

        trash[n_trash].y +=
            (int)(trash[n_trash].velocity.amplitude *
                  sinf(trash[n_trash].velocity.angle));

        correct_position(&trash[n_trash].x);
        correct_position(&trash[n_trash].y);
    }
}

/* ------------------------------------------------------------
   Atualiza posição das ships
   ------------------------------------------------------------ */
void new_ship_position(ship ships[], int total_ships)
{
    for (int s = 0; s < total_ships; s++){
        ships[s].x +=
            ships[s].velocity.amplitude *
            cosf(ships[s].velocity.angle);

        ships[s].y +=
            ships[s].velocity.amplitude *
            sinf(ships[s].velocity.angle);

        correct_position_f(&ships[s].x);
        correct_position_f(&ships[s].y);
    }
}

/* ------------------------------------------------------------
   Aplica thrust manual à ship
   ------------------------------------------------------------
   Adiciona um pequeno vetor de velocidade na direção pedida
   ------------------------------------------------------------ */
void new_ship_thrust(ship *s, direction_t d)
{
    vector thrust;
    thrust.amplitude = 0.5f;

    switch (d) {
        case UP:    thrust.angle = -M_PI / 2; break;
        case DOWN:  thrust.angle =  M_PI / 2; break;
        case LEFT:  thrust.angle =  M_PI;     break;
        case RIGHT: thrust.angle =  0;        break;
        default:    return;
    }

    s->velocity = add_vectors(s->velocity, thrust);
}
