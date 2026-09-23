
#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include "structs.h"

// ============================================================================
// Create ZMQ server socket
// ----------------------------------------------------------------------------
// Creates a ZMQ REP socket and binds it to port 5555.
//
// Returns:
//   Pointer to the created server socket (void*).
// ============================================================================
void * create_server_channel(void);

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
void * create_client_channel(char * server_ip_addr);

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

#endif

