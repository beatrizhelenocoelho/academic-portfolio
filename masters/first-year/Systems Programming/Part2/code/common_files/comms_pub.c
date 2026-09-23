#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "structs.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>  
#include "message_comms.pb-c.h"
#include <zmq.h>

/////////////////////////////////
///Creates and binds a ZMQ PUB socket
/////////////////////////////////
void *create_pub_channel(void *ctx, const char * pub_port) {
    void *pub = zmq_socket(ctx, ZMQ_PUB);
 if (!pub) {
        perror("zmq_socket");
        return NULL;
    }

    char endpoint[64];
    snprintf(endpoint, sizeof(endpoint), "tcp://*:%s", pub_port);

    zmq_bind(pub, endpoint);  // separate port for PUB
    usleep(200 );             // allow subscribers to connect
    return pub;
}

/////////////////////////////////
///Creates and connects a ZMQ SUB socket
/////////////////////////////////

void *create_sub_channel(const char *server_ip_addr, void *ctx, const char *port) {
    char zmq_addr[128];
    sprintf(zmq_addr, "tcp://%s:%s", server_ip_addr, port);

    void *subscriber = zmq_socket(ctx, ZMQ_SUB);
    if (!subscriber) {
        perror("zmq_socket");
        return NULL;
    }

    /* ---- ADD THIS ---- */
    int timeout_ms = 500; // 0.5 second
    zmq_setsockopt(subscriber, ZMQ_RCVTIMEO,
                   &timeout_ms, sizeof(timeout_ms));
    /* ------------------ */

    // Subscribe to all messages
    if (zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0) != 0) {
        perror("zmq_setsockopt");
        zmq_close(subscriber);
        return NULL;
    }

    if (zmq_connect(subscriber, zmq_addr) != 0) {
        perror("zmq_connect");
        zmq_close(subscriber);
        return NULL;
    }

    return subscriber;
}

/////////////////////////////////
///Publishes world state to all subscribers
/////////////////////////////////
int send_world_state_pub(
    void *zmq_pub_socket,                   // PUB socket
    const planet_structure *planets, size_t n_planets,
    const trash_structure  *trash,   size_t n_trash,
    const ship             *ships,   size_t n_ships,
    int seq)
{
    size_t i;
    int ret = -1;

    Planet **pb_planets = calloc(n_planets, sizeof(*pb_planets));
    Trash  **pb_trash   = calloc(n_trash,   sizeof(*pb_trash));
    Ship   **pb_ships   = calloc(n_ships,   sizeof(*pb_ships));

    if (!pb_planets || !pb_trash || !pb_ships)
        goto cleanup_arrays;

    /* ---------- PLANETS ---------- */
    for (i = 0; i < n_planets; i++) {
        Planet *p = malloc(sizeof(*p));
        Color  *c = malloc(sizeof(*c));
        if (!p || !c)
            goto cleanup_objects;

        *p = (Planet) PLANET__INIT;
        *c = (Color)  COLOR__INIT;

        p->name  = strdup(planets[i].name);
        p->x     = planets[i].x;
        p->y     = planets[i].y;
        p->trash = planets[i].trash;
        p->mass  = planets[i].mass;

        c->r = planets[i].color->r;
        c->g = planets[i].color->g;
        c->b = planets[i].color->b;
        c->a = planets[i].color->a;

        p->color = c;
        pb_planets[i] = p;
    }

    /* ---------- TRASH ---------- */
    for (i = 0; i < n_trash; i++) {
        Trash *t = malloc(sizeof(*t));
        Color *c = malloc(sizeof(*c));
        if (!t || !c)
            goto cleanup_objects;

        *t = (Trash) TRASH__INIT;
        *c = (Color) COLOR__INIT;

        t->x     = trash[i].x;
        t->y     = trash[i].y;
        t->mass  = trash[i].mass;
        t->taken = trash[i].taken;

        if (trash[i].taken)
            t->ship_name = strdup(trash[i].ship_name);
        else
            t->ship_name = NULL;

        c->r = trash[i].color->r;
        c->g = trash[i].color->g;
        c->b = trash[i].color->b;
        c->a = trash[i].color->a;

        t->color = c;
        pb_trash[i] = t;
    }

    /* ---------- SHIPS ---------- */
    for (i = 0; i < n_ships; i++) {
        Ship *s = malloc(sizeof(*s));
        if (!s)
            goto cleanup_objects;

        *s = (Ship) SHIP__INIT;

        s->x     = ships[i].x;
        s->y     = ships[i].y;
        s->name  = strdup(ships[i].name);
        s->use   = ships[i].use;
        s->trash = ships[i].trash;

        pb_ships[i] = s;
    }

    /* ---------- WORLD STATE ---------- */
    WorldState world = WORLD_STATE__INIT;
    world.seq = seq;
    world.n_planets = n_planets;
    world.planets   = pb_planets;
    world.n_trash   = n_trash;
    world.trash     = pb_trash;
    world.n_ships   = n_ships;
    world.ships     = pb_ships;

    /* ---------- SERIALIZE ---------- */
    size_t size = protobuf_c_message_get_packed_size(&world.base);
    uint8_t *buffer = malloc(size);
    if (!buffer)
        goto cleanup_objects;

    protobuf_c_message_pack(&world.base, buffer);

    /* ---------- SEND VIA PUB SOCKET ---------- */
    // Send topic first
    zmq_send(zmq_pub_socket, "WORLD", 5, ZMQ_SNDMORE);
    // Send the serialized protobuf
    if (zmq_send(zmq_pub_socket, buffer, size, 0) == (int)size)
        ret = 0;

    free(buffer);

cleanup_objects:
    for (i = 0; i < n_planets; i++) {
        if (pb_planets && pb_planets[i]) {
            free(pb_planets[i]->name);
            free(pb_planets[i]->color);
            free(pb_planets[i]);

        }
    }

    for (i = 0; i < n_trash; i++) {
        if (pb_trash && pb_trash[i]) {
            if (pb_trash[i]->ship_name)
                free(pb_trash[i]->ship_name);
            free(pb_trash[i]->color);
            free(pb_trash[i]);

        }
    }

    for (i = 0; i < n_ships; i++) {
        if (pb_ships && pb_ships[i]) {
            free(pb_ships[i]->name);
            free(pb_ships[i]);
        }
    }


cleanup_arrays:
    free(pb_planets);
    free(pb_trash);
    free(pb_ships);

    return ret;
}



/////////////////////////////////
///Receives and decodes WORLD state update
///
///Receives a multipart ZMQ message (topic + protobuf),
///unpacks the WorldState protobuf, discards old packets,
///and copies planets, trash, and ships into local structures.
///
///Special return values:
///  0  - success
///  5  - server shutdown signal (seq == -1)
///  6  - world reset signal (seq == 0)
/// -1  - ZMQ receive error
/// -2  - invalid or outdated packet
/////////////////////////////////




int receive_world_state(void *sub_socket,
                        planet_structure **planets_out, size_t *n_planets,
                        trash_structure  **trash_out,  size_t *n_trash,
                        ship             **ships_out,  size_t *n_ships, int *seq) {

    // --- 1. Receive topic frame ---
    char topic[32];
    int topic_size = zmq_recv(sub_socket, topic, sizeof(topic)-1, 0);
    if (topic_size <= 0) return -1;
    topic[topic_size] = '\0';

    int more = 0;
    size_t more_size = sizeof(more);
    zmq_getsockopt(sub_socket, ZMQ_RCVMORE, &more, &more_size);
    if (!more) return -1; // must have a second frame

    // --- 2. Receive protobuf frame ---
    uint8_t buffer[65536]; 
    int n = zmq_recv(sub_socket, buffer, sizeof(buffer), 0);
    if (n <= 0) return -1;

    WorldState *world = world_state__unpack(NULL, n, buffer);
    if (!world) return -2;

    // --- 3. Discard old packets ---
    if(world->seq == -1)
        return 5;
    if(world->seq == 0)
        return 6;

    if (world->seq <= *seq) {
        world_state__free_unpacked(world, NULL);
        return -2;
    }
    *seq = world->seq;

    // --- 4. Copy planets ---
    *n_planets = world->n_planets;
    *planets_out = calloc(*n_planets, sizeof(planet_structure));
    for (size_t i = 0; i < *n_planets; i++) {
        Planet *p = world->planets[i];
        (*planets_out)[i].x = p->x;
        (*planets_out)[i].y = p->y;
        (*planets_out)[i].trash = p->trash;
        (*planets_out)[i].mass = p->mass;
        strncpy((*planets_out)[i].name, p->name, sizeof((*planets_out)[i].name));
        (*planets_out)[i].color = malloc(sizeof(SDL_Color));
        (*planets_out)[i].color->r = p->color->r;
        (*planets_out)[i].color->g = p->color->g;
        (*planets_out)[i].color->b = p->color->b;
        (*planets_out)[i].color->a = p->color->a;
    }

    // --- 5. Copy trash ---
    *n_trash = world->n_trash;
    *trash_out = calloc(*n_trash, sizeof(trash_structure));
    for (size_t i = 0; i < *n_trash; i++) {
        Trash *t = world->trash[i];
        (*trash_out)[i].x = t->x;
        (*trash_out)[i].y = t->y;
        (*trash_out)[i].mass = t->mass;
        (*trash_out)[i].taken = t->taken;
        if (t->ship_name)
            strncpy((*trash_out)[i].ship_name, t->ship_name, sizeof((*trash_out)[i].ship_name));
        else
            (*trash_out)[i].ship_name[0] = '\0';

        (*trash_out)[i].color = malloc(sizeof(SDL_Color));
        (*trash_out)[i].color->r = t->color->r;
        (*trash_out)[i].color->g = t->color->g;
        (*trash_out)[i].color->b = t->color->b;
        (*trash_out)[i].color->a = t->color->a;
    }

    // --- 6. Copy ships ---
    *n_ships = world->n_ships;
    *ships_out = calloc(*n_ships, sizeof(ship));
    for (size_t i = 0; i < *n_ships; i++) {
        Ship *s = world->ships[i];
        (*ships_out)[i].x = s->x;
        (*ships_out)[i].y = s->y;
        (*ships_out)[i].use  = s->use;
        (*ships_out)[i].trash = s->trash;
        strncpy((*ships_out)[i].name, s->name, sizeof((*ships_out)[i].name));
    }

    world_state__free_unpacked(world, NULL);
    return 0;
}

