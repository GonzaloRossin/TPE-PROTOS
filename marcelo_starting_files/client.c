#include "client.h"


void
new_client(struct socks5 * newClient, int clientSocket, int BUFFSIZE){
    newClient->client_socket = clientSocket;

    buffer newBuffer;

    uint8_t * data = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    memset(data, 0, sizeof(uint8_t) * BUFFSIZE);
    buffer_init(&newBuffer, BUFFSIZE, data);

    newClient->bufferFromClient = newBuffer;

    newClient->isAvailable = false;

    
}

void init_client_copy(struct socks5 * client) {
    struct copy st_copy = (client->client.st_copy);

    client->client.st_copy.write_fd = client->remote_socket;
    client->client.st_copy.read_fd = client->client_socket;
    client->client.st_copy.r = client->bufferFromClient;
    client->client.st_copy.w = client->bufferFromRemote;
    client->connection_state.init = 1;
}

void
set_client_remote(struct socks5 * client, int remote_socket, int BUFFSIZE){
    client->remote_socket = remote_socket;

    buffer newBuffer;

    uint8_t * data = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    memset(data, 0, sizeof(uint8_t) * BUFFSIZE);
    buffer_init(&newBuffer, BUFFSIZE, data);

    client->bufferFromRemote = newBuffer;
}

void removeClient(struct socks5 * client){
    close( client->client_socket );
    close( client->remote_socket );

    client->client_socket = 0;
    client->remote_socket = 0;
    client->isAvailable = true;
    free(client->bufferFromClient.data);
    free(client->bufferFromRemote.data);
}



