/* Client.h

to manage clients and their important data

server has array of clients

*/

#ifndef CLIENT_H_
#define CLIENT_H_

#include "buffer.h"
#include "logger.h"
#include "hello.h"
#include "request.h"
#include <stdbool.h>
#include <unistd.h>  // size_t, ssize_t
#include <stdio.h>
#include <stdlib.h>
#include "selector.h"
#include "auth_parser.h"

#define IP_V4_ADDR_SIZE 4
#define IP_V6_ADDR_SIZE 16

enum client_state {
    HELLO_READ_STATE = 0,
    HELLO_WRITE_STATE,
    UP_READ_STATE,
    UP_WRITE_STATE,
    REQUEST_READ_STATE,
    REQUEST_RESOLVE_STATE,
    REQUEST_CONNECTING_STATE,
    REQUEST_WRITE_STATE,
    CONNECTED_STATE,
    CLOSE_STATE,
    ERROR_STATE,
};

typedef enum t_protocol{
    PROT_UNIDENTIFIED,
    PROT_POP3
} t_protocol;

typedef enum AuthStatus
{
    AUTH_SUCCESS = 0x00,
    AUTH_FAILURE = 0xFF,
} AuthStatus;

struct clients_data
{
    struct socks5 * clients;
    int clients_size;
};

struct hello
{
    hello_parser * pr;
    buffer * w;
    buffer * r;

    uint8_t method;
};

typedef struct userpass_st {
    buffer *r, *w;

    struct up_req_parser * parser;

    uint8_t * user;
    uint8_t * password;
} userpass_st;

struct st_request
{
    request_parser * pr;
    buffer * r;
    buffer * w;
    enum socks_response_status state;
    struct request * request;
    int origin_fd;
};

typedef struct connected {
  int fd;

  fd_interest interest;

  struct connected * other_connected;

  struct pop3_parser * pop_parser;
  buffer * aux_b;

  buffer * w;
  buffer * r;
} connected;

typedef struct connection_state {
    int init;
    enum client_state client_state;
    void (*on_departure) (struct socks5 * currClient);
    void (*on_arrival) (struct socks5 * currClient);
} connection_state;

struct connecting {
    buffer *w;
    enum client_state client_state;
    int origin_fd;
};

struct socks5
{
    int client_socket;
    int remote_socket;

    int origin_domain;
    
    struct addrinfo *origin_resolution;
    struct addrinfo *origin_resolution_current;

    int origin_adrr_type;
    int origin_addr_len;
    uint8_t origin_ipv4_addr[IP_V4_ADDR_SIZE];
    uint8_t origin_port[2];
    t_protocol protocol_type;

    struct sockaddr_storage origin_addr;

    buffer * bufferFromClient;
    buffer * bufferFromRemote;

    struct connection_state * connection_state;
    bool disector_enabled;

    union {
        struct hello st_hello;
        struct userpass_st userpass;
        struct st_request st_request;
        struct connected st_connected;
    } client;

    union {
        struct connecting conn;
        connected st_connected;
    } remote;

    uint8_t * username;

    bool isAvailable;
};

//creates new client
void
new_client(struct socks5 * newClient, int clientSocket, int BUFFSIZE);

//frees client resources
void removeClient(struct socks5 * client);

enum client_state
advanceClientState();

void
set_client_remote(struct socks5 * client, int remote_socket, int BUFFSIZE);


#endif