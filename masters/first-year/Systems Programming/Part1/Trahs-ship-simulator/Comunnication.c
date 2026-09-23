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

//////////////////////////////////
///Creates ZMQ REP server socket bound to port 5555
/////////////////////////////////
void * create_server_channel(){
    void *context = zmq_ctx_new ();
    void *responder = zmq_socket (context, ZMQ_REP);
    int response = zmq_bind (responder, "tcp://*:5555");

    if (response != 0) {
    printf("Error binding socket: %s\n", zmq_strerror(errno));
    }
    return responder;
}


//////////////////////////////////
///Receives ZMQ message and decodes client movement/connection protobuf (ON SERVER)
/////////////////////////////////
void read_message(void *fd, char *message_type, char *c,
                  direction_t *d, int *pass_out)
{
    uint8_t buffer[256];
    int n = zmq_recv(fd, buffer, sizeof(buffer), ZMQ_DONTWAIT);

    if (n == -1 && errno == EAGAIN) {
        *message_type = '\0';
        return;
    }

    *message_type = '\0';
    *c = '\0';
    if (d) *d = 0;
    if (pass_out) *pass_out = 0;

    if (n <= 0)
        return;

    // Try to decode as client_movement_req
    ClientMovementReq *move_req =
        client_movement_req__unpack(NULL, n, buffer);

    if (move_req != NULL) {

        // 1. message type (protobuf field 1)
        if (move_req->type != NULL)
            strcpy(message_type, move_req->type);
        else
            strcpy(message_type, "MOVE");

        // 2. ch (bytes → your single-character name)
        if (move_req->ch.len > 0)
            *c = move_req->ch.data[0];
        else
            *c = '\0';

        // 3. pass field
        if (pass_out)
            *pass_out = move_req->pass;

        // 4. direction
        if (d)
            *d = (direction_t)(move_req->dir);

        client_movement_req__free_unpacked(move_req, NULL);
        return;
    }

    // Try to decode as client_connection_req
    ClientConnectionReq *conn_req =
        client_connection_req__unpack(NULL, n, buffer);

    if (conn_req != NULL) {
        strcpy(message_type, "HELLO");
        client_connection_req__free_unpacked(conn_req, NULL);
        return;
    }

    // Unknown message
    strcpy(message_type, "UNKNOWN");
}

//////////////////////////////////
///Sends movement result to client using protobuf over ZMQ
/////////////////////////////////
void send_response (void * fd,  respost rsp){
    ServerMovementReq msg = SERVER_MOVEMENT_REQ__INIT;

    // Map your enum to protobuf MoveResult
    if (rsp == RES_OK) {
        msg.result = MOVE_RESULT__OK;      // assuming MoveResult enum in protobuf
    } else {
        msg.result = MOVE_RESULT__WALL;    // or MOVE_RESULT__ERROR if you have it
    }

    // Pack protobuf into buffer
    size_t size = server_movement_req__get_packed_size(&msg);
    uint8_t buffer[32]; // small buffer, enough for this message
    server_movement_req__pack(&msg, buffer);

    // Send packed message via ZMQ
    zmq_send(fd, buffer, size, 0);
}


//////////////////////////////////
///Sends connection response with name and pass to client
/////////////////////////////////
void send_response_connection(void *fd, respost rsp, ship *player) {
    ServerConnectionReq msg = SERVER_CONNECTION_REQ__INIT;

    // Set result field
    if (rsp == RES_OK)
        msg.result = CONNECTION_RESULT__CONN_OK;
    else
        msg.result = CONNECTION_RESULT__CONN_ERROR;

    // Set name field (bytes) from ship->name
    msg.name.data = (uint8_t *)&player->name[0];  // single char
    msg.name.len  = 1;  // only 1 byte

    // Set pass field from ship
    msg.pass = player->pass;

    // Pack protobuf
    size_t size = server_connection_req__get_packed_size(&msg);

    uint8_t buffer[256];  // large enough for name + header
    server_connection_req__pack(&msg, buffer);

    // Send via ZMQ
    zmq_send(fd, buffer, size, 0);
}




//////////////////////////////////
///Creates ZMQ REQ client socket and connects to server
/////////////////////////////////
void * create_client_channel(char * server_ip_addr){
    char server_zmq_addr[100];
    sprintf(server_zmq_addr, "tcp://%s:5555", server_ip_addr);
    void *context = zmq_ctx_new ();
    void *requester = zmq_socket (context, ZMQ_REQ);
    zmq_connect (requester, "tcp://localhost:5555");

    return requester;
}


//////////////////////////////////
///Sends HELLO connection request protobuf to server
/////////////////////////////////
void send_connection_message(void *fd) {
    ClientConnectionReq req = CLIENT_CONNECTION_REQ__INIT;

    // Explicitly set the field so the packer writes it
    req.type = "HELLO";

    size_t size = client_connection_req__get_packed_size(&req);
    if (size == 0) {
        fprintf(stderr, "send_connection_message: packed size is 0 — nothing to send\n");
        return;
    }

    uint8_t *buffer = malloc(size);
    if (!buffer) { perror("malloc"); return; }
    client_connection_req__pack(&req, buffer);

    int sent = zmq_send(fd, buffer, size, 0);
    if (sent == -1) perror("zmq_send");

    free(buffer);
}


//////////////////////////////////
///Sends movement request with name, pass, direction over ZMQ
/////////////////////////////////
void send_movement_message(void *fd, ship *player, direction_t d) {
    ClientMovementReq req = CLIENT_MOVEMENT_REQ__INIT;

    // Optional string "type" → default "MOVE" is used automatically

    // Set ch as a single byte from ship->name
    req.ch.data = (uint8_t *)&player->name[0];
    req.ch.len  = 1;

    // Set pass from ship struct
    req.pass = player->pass;

    // Set direction (map enum)
    req.dir = (DirectionEnum)d;

    // Pack protobuf
    size_t size = client_movement_req__get_packed_size(&req);

    uint8_t buffer[32];  // enough for this small message
    client_movement_req__pack(&req, buffer);

    // Send message via ZMQ
    zmq_send(fd, buffer, size, 0);
}



//////////////////////////////////
///Receives and interprets server movement response protobuf (ON CLIENT)
/////////////////////////////////
int receive_response (void * fd){
    
    uint8_t buffer[128];
    int n = zmq_recv(fd, buffer, sizeof(buffer), 0);

    ServerMovementReq *msg = server_movement_req__unpack(NULL, n, buffer);
    if (msg == NULL) {
        // Failed to unpack
        return 0;
    }

    // Map protobuf MoveResult to your respost enum
    switch (msg->result) {
        case MOVE_RESULT__OK:
            return 1;
             
        case MOVE_RESULT__WALL:
        default:
            
            return 0;
    }
}

//////////////////////////////////
///Receives connection response and ship credentials
/////////////////////////////////
int receive_response_connection(void *fd, ship *ship) {
    uint8_t buffer[256];  // buffer to receive message
    int n = zmq_recv(fd, buffer, sizeof(buffer), 0);
    if (n <= 0) {
        // Receive error
        return 0;
    }

    // Unpack protobuf
    ServerConnectionReq *msg = server_connection_req__unpack(NULL, (size_t)n, buffer);
    if (msg == NULL) {
        // Failed to unpack
        return 0;
    }

    // Store name as single char
    if (msg->name.len > 0) {
        ship->name[0] = msg->name.data[0];
        ship->name[1] = '\0';  // null terminate
    } else {
        ship->name[0] = '\0';  // fallback if no data
    }


    // Store pass
    ship->pass = msg->pass;

    // Map result
    int result;
    if (msg->result == CONNECTION_RESULT__CONN_OK)
        result = 1;
    else
        result = 0;

    // Free protobuf
    server_connection_req__free_unpacked(msg, NULL);

    return result;
}
