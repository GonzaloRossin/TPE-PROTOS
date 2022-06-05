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
    socks_hello_state,
    socks_request_state,
    connected_state,
    close_state,
    error_state,
};

struct clients_data
{
    struct client * clients;
    int clients_size;
};

struct client
{
    int client_socket;
    int remote_socket;

    struct buffer bufferFromClient;
    struct buffer bufferFromRemote;

    enum client_state state;

    bool isAvailable;
};

//creates new client
void
new_client(struct client * newClient, int clientSocket, int BUFFSIZE);

//frees client resources
void
removeClient();

enum client_state
advanceClientState();

void
set_client_remote(struct client * client, int remote_socket, int BUFFSIZE);


