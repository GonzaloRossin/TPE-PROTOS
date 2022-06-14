#include "./../../include/socks5.h"


void
new_client(struct socks5 * newClient, int clientSocket, int BUFFSIZE){
    newClient->client_socket = clientSocket;

    buffer * newClientBuffer = (buffer*)malloc(sizeof(buffer));

    uint8_t * dataClient = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    memset(dataClient, 0, sizeof(uint8_t) * BUFFSIZE);
    buffer_init(newClientBuffer, BUFFSIZE, dataClient);

    newClient->bufferFromClient = newClientBuffer;

    buffer * newOriginBuffer = (buffer*)malloc(sizeof(buffer));
    uint8_t * dataOrigin = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    memset(dataOrigin, 0, sizeof(uint8_t) * BUFFSIZE);
    buffer_init(newOriginBuffer, BUFFSIZE, dataOrigin);

    newClient->bufferFromRemote = newOriginBuffer;

    newClient->isAvailable = false;

}

void
set_client_remote(struct socks5 * client, int remote_socket, int BUFFSIZE){
    client->remote_socket = remote_socket;

    buffer * newBuffer = (buffer*)malloc(sizeof(buffer));

    uint8_t * data = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    memset(data, 0, sizeof(uint8_t) * BUFFSIZE);
    buffer_init(newBuffer, BUFFSIZE, data);

    client->bufferFromRemote = newBuffer;
}

void removeClient(struct socks5 * client) {
    close( client->client_socket );

    client->client_socket = 0;
    client->remote_socket = 0;
    client->isAvailable = true;
    free(client->bufferFromClient->data);
    free(client->bufferFromRemote->data);
}



