
#ifndef CLIENT_H
#define CLIENT_H
/* Client.h

to manage clients and their important data

server has array of clients

*/
// #include "buffer.h"
// #include "logger.h"
#include "hello.h"
#include "request.h"
// #include <stdbool.h>
// #include <unistd.h>  // size_t, ssize_t
// #include <stdio.h>
// #include <stdlib.h>
#include "proxyHandler.h"
enum client_state {
    hello_read_state = 0,
    hello_write_state,
    request_read_state,
    connected_state,
    close_state,
    error_state,
};

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

struct st_request
{
    request_parser * pr;
    buffer * r;
};

struct connected {
  int write_fd;
  int read_fd;

  buffer * w;
  buffer * r;
  int init;
};

struct connection_state {
    int init;
    enum client_state client_state;
};

struct socks5
{
    int client_socket;
    int remote_socket;

    buffer * bufferFromClient;
    buffer * bufferFromRemote;

    struct connection_state connection_state;

    union {
        struct hello st_hello;
        struct st_request st_request;
        struct connected st_connected;
    } client;

    union {
        struct connected st_connected;
    } remote;

    bool isAvailable;
};

//creates new client
void
new_client(struct socks5 * newClient, int clientSocket, int BUFFSIZE);

void init_client_copy(struct socks5 * client);
void init_remote_copy(struct socks5 * client);

//frees client resources
void removeClient(struct socks5 * client);

enum client_state
advanceClientState();

void
set_client_remote(struct socks5 * client, int remote_socket, int BUFFSIZE);


#endif