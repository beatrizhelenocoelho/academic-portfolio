
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "structs.h"
#include <stdint.h>

// ============================================================================
// Create ZMQ server socket
// ----------------------------------------------------------------------------
// Creates a ZMQ REP socket and binds it to port 5555.
//
// Returns:
//   Pointer to the created server socket (void* ctx).
// ============================================================================
void *create_server_channel(void *ctx, const char *port);

// ============================================================================
// Send movement response to client
// ----------------------------------------------------------------------------
// Sends the result of a movement action (RES_OK / RES_WALL) via protobuf.
//
// Parameters:
//   fd  - ZMQ socket to send on
//   rsp - movement result enum (respost)
// ============================================================================
void send_response(void *fd, respost rsp);

// ============================================================================
// Send connection response to client
// ----------------------------------------------------------------------------
// Sends ship name and password back to client upon connection attempt.
//
// Parameters:
//   fd     - ZMQ socket to send on
//   rsp    - connection result (RES_OK / RES_ERROR)
//   player - ship whose credentials are sent
// ============================================================================
void send_response_connection(void *fd, respost rsp, ship* player);

// ============================================================================
// Create ZMQ client socket
// ----------------------------------------------------------------------------
// Creates a ZMQ REQ socket and connects it to the server at given IP.
//
// Parameters:
//   server_ip_addr - string containing server IP address
//
// Returns:
//   Pointer to created client socket (void*).
// ============================================================================
void * create_client_channel(const char * server_ip_addr, void *ctx, const char *port);

// ============================================================================
// Send movement request to server
// ----------------------------------------------------------------------------
// Sends the ship's name, password, and direction to server via protobuf.
//
// Parameters:
//   fd     - ZMQ socket to send on
//   player - ship sending the movement
//   d      - direction of movement
// ============================================================================
void send_movement_message(void * fd, ship* player, direction_t d);

// ============================================================================
// Send HELLO connection request to server
// ----------------------------------------------------------------------------
// Sends a client connection request (HELLO) to server.
//
// Parameters:
//   fd - ZMQ socket to send on
// ============================================================================
void send_connection_message(void *fd);

// ============================================================================
// Receive movement response from server (client-side)
// ----------------------------------------------------------------------------
// Receives and decodes a server movement response protobuf.
//
// Parameters:
//   fd - ZMQ socket to receive from
//
// Returns:
//   1 if movement was successful, 0 if blocked or invalid.
// ============================================================================
int receive_response(void *fd);

// ============================================================================
// Receive connection response from server (client-side)
// ----------------------------------------------------------------------------
// Receives connection response and updates local ship credentials.
//
// Parameters:
//   fd   - ZMQ socket to receive from
//   ship - pointer to ship struct to update name and password
//
// Returns:
//   1 if connection succeeded, 0 otherwise.
// ============================================================================
int receive_response_connection(void *fd, ship *ship);

// ============================================================================
// Read message from client (server-side)
// ----------------------------------------------------------------------------
// Receives a ZMQ message and decodes it as either movement or connection request.
//
// Parameters:
//   fd           - ZMQ socket to receive from
//   message_type - output string, will contain "MOVE", "HELLO", or "UNKNOWN"
//   c            - output character for ship name (if movement)
//   d            - output direction (if movement), can be NULL
//   pass_out     - output password (if movement), can be NULL
// ============================================================================
void read_message(void *fd, char *message_type, char *c,
                  direction_t *d, int *pass_out);


// ============================================================================
// Send ACTIVE message to server (client-side)
// ----------------------------------------------------------------------------
// Sends a keep-alive message to inform the server that the ship
// is still active. This prevents the server from deactivating the ship due
// to inactivity.
//
// Parameters:
//   fd     - ZMQ socket used to send the message
//   player - pointer to the player's ship structure
// ============================================================================
void send_active_message(void *fd, ship *player);


// ============================================================================
// Send EXIT message to server (client-side)
// ----------------------------------------------------------------------------
// Sends a request to notify the server that the client is disconnecting
// and that the associated ship should be deactivated.
//
// Parameters:
//   fd     - ZMQ socket used to send the message
//   player - pointer to the player's ship structure
// ============================================================================
void send_exit_message(void *fd, ship *player);



// ============================================================================
// Create PUB socket and bind to port (server-side)
// ----------------------------------------------------------------------------
// Creates a ZMQ PUB socket, binds it to the given port, and waits briefly
// to allow subscribers to connect.
//
// Parameters:
//   ctx       - ZMQ context
//   pub_port  - port string (e.g., "5556")
//
// Returns:
//   Pointer to ZMQ PUB socket, or NULL on failure
// ============================================================================
void *create_pub_channel(void *ctx, const char *pub_port);


// ============================================================================
// Create SUB socket and connect to PUB server (client-side)
// ----------------------------------------------------------------------------
// Creates a ZMQ SUB socket, connects to the given server address and port,
// sets a receive timeout, and subscribes to all topics.
//
// Parameters:
//   server_ip_addr - server IP or hostname
//   ctx            - ZMQ context
//   port           - port string (e.g., "5556")
//
// Returns:
//   Pointer to ZMQ SUB socket, or NULL on failure
// ============================================================================
void *create_sub_channel(const char *server_ip_addr,
                         void *ctx,
                         const char *port);


// ============================================================================
// Publish world state using ZMQ PUB (server-side)
// ----------------------------------------------------------------------------
// Serializes the current world state (planets, trash, ships) using protobuf
// and publishes it over a ZMQ PUB socket under the "WORLD" topic.
//
// Parameters:
//   zmq_pub_socket - PUB socket
//   planets        - array of planet structures
//   n_planets      - number of planets
//   trash          - array of trash structures
//   n_trash        - number of trash objects
//   ships          - array of ship structures
//   n_ships        - number of ships
//   seq            - sequence number of the world update
//
// Returns:
//   0 on success, -1 on failure
// ============================================================================
int send_world_state_pub(
    void *zmq_pub_socket,
    const planet_structure *planets, size_t n_planets,
    const trash_structure  *trash,   size_t n_trash,
    const ship             *ships,   size_t n_ships,
    int seq
);


// ============================================================================
// Receive and decode world state from PUB server (client-side)
// ----------------------------------------------------------------------------
// Receives a multipart ZMQ message containing the "WORLD" topic followed by
// a serialized WorldState protobuf message. The world state is unpacked and
// copied into newly allocated structures.
//
// Parameters:
//   sub_socket   - ZMQ SUB socket
//   planets_out  - output pointer to allocated planet array
//   n_planets    - output number of planets
//   trash_out    - output pointer to allocated trash array
//   n_trash      - output number of trash objects
//   ships_out    - output pointer to allocated ship array
//   n_ships      - output number of ships
//   seq          - input/output sequence number (used to discard old packets)
//
// Returns:
//   0  on success
//   5  if server closed normally (seq == -1)
//   6  if world collapsed (seq == 0)
//  -1  on receive error
//  -2  on invalid or out-of-order packet
// ============================================================================
int receive_world_state(
    void *sub_socket,
    planet_structure **planets_out, size_t *n_planets,
    trash_structure  **trash_out,   size_t *n_trash,
    ship             **ships_out,   size_t *n_ships,
    int *seq
);
#endif

