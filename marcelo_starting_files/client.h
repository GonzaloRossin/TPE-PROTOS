/* Client.h

to manage clients and their important data

server has array of clients

*/
#include "buffer.h"
#include "logger.h"
#include <stdbool.h>
#include <unistd.h>  // size_t, ssize_t
#include <stdio.h>
#include <stdlib.h>

enum client_state {
    socks_hello_state = 0,
    socks_request_state,
    connected_state,
    close_state,
    error_state,
};

struct clients_data
{
    struct socks5 * clients;
    int clients_size;
};

struct copy {
  int write_fd;
  struct buffer w;
  struct buffer r;
  int read_fd;
};

struct connection_state {
    int init;
    enum client_state client_state;
};

struct socks5
{
    int client_socket;

    int remote_socket;

    struct buffer bufferFromClient;
    struct buffer bufferFromRemote;

    struct connection_state connection_state;

    union {
        struct copy st_copy;
    } client;

    union {
        struct copy st_copy;
    } remote;

    bool isAvailable;
};

//creates new client
void
new_client(struct socks5 * newClient, int clientSocket, int BUFFSIZE);

void init_client_copy(struct socks5 * client);

//frees client resources
void removeClient(struct socks5 * client);

enum client_state
advanceClientState();

void
set_client_remote(struct socks5 * client, int remote_socket, int BUFFSIZE);


