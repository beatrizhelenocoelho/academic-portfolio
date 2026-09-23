#include <math.h>
#include "../structs.h"  

static int UNIVERSE_SIZE = 800;

void set_universe_size(int size) {
    UNIVERSE_SIZE = size;
}


static vector make_vector(float dx, float dy) {
    vector v;
    v.amplitude = sqrtf(dx * dx + dy * dy);
    v.angle = atan2f(dy, dx);
    return v;
}

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

static void correct_position(int *coord) {
    if (*coord < 0) {
        *coord += UNIVERSE_SIZE;
    } else if (*coord >= UNIVERSE_SIZE) {
        *coord -= UNIVERSE_SIZE;
    }
}


void new_trash_acceleration(planet_structure planets[], int total_planets,
                            trash_structure trash[], int total_trash){
    vector total_vector_force;

    for (int n_trash = 0; n_trash < total_trash; n_trash++){
        total_vector_force.amplitude = 0;
        total_vector_force.angle = 0;

        for (int n_planet = 0; n_planet < total_planets; n_planet++){
            float force_vector_x = (float)planets[n_planet].x - (float)trash[n_trash].x;
            float force_vector_y = (float)planets[n_planet].y - (float)trash[n_trash].y;

            vector local_vector_force = make_vector(force_vector_x, force_vector_y);

            if (local_vector_force.amplitude == 0) {
                continue; 
            }

            local_vector_force.amplitude =
                50*(planets[n_planet].mass * trash[n_trash].mass) /      // o lixo não se estava a mexer então precisava de uma constante para aumentar a atração G 
                powf(local_vector_force.amplitude, 2);

            total_vector_force = add_vectors(local_vector_force, total_vector_force);
        }

        trash[n_trash].acceleration = total_vector_force; 
    }
}

void new_trash_velocity(trash_structure trash[], int total_trash){
    for (int n_trash = 0; n_trash < total_trash; n_trash++){
        trash[n_trash].velocity.amplitude *= 0.99f; 
        trash[n_trash].velocity =
            add_vectors(trash[n_trash].velocity, trash[n_trash].acceleration);
    }
}

void new_trash_position(trash_structure trash[], int total_trash){
    for (int n_trash = 0; n_trash < total_trash; n_trash++){
        trash[n_trash].x += (int)(trash[n_trash].velocity.amplitude *
                                  cosf(trash[n_trash].velocity.angle));
        trash[n_trash].y += (int)(trash[n_trash].velocity.amplitude *
                                  sinf(trash[n_trash].velocity.angle));

        correct_position(&trash[n_trash].x);
        correct_position(&trash[n_trash].y);
    }
}
