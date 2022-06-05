#include "./include/client.h"


void
new_client(struct client * newClient, int clientSocket, int BUFFSIZE){
    newClient->client_socket = clientSocket;

    buffer newBuffer;

    uint8_t * data = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    memset(data, 0, sizeof(uint8_t) * BUFFSIZE);
    buffer_init(&newBuffer, BUFFSIZE, data);

    newClient->bufferFromClient = newBuffer;

    newClient->isAvailable = false;
}

void
set_client_remote(struct client * client, int remote_socket, int BUFFSIZE){
    client->remote_socket = remote_socket;

    buffer newBuffer;

    uint8_t * data = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    memset(data, 0, sizeof(uint8_t) * BUFFSIZE);
    buffer_init(&newBuffer, BUFFSIZE, data);

    client->bufferFromRemote = newBuffer;
}

void removeClient(struct client * client){
    close( client->client_socket );
    close( client->remote_socket );

    client->client_socket = 0;
    client->remote_socket = 0;
    client->isAvailable = true;
    free(client->bufferFromClient.data);
    free(client->bufferFromRemote.data);
}



