#include "./../../include/ssemd.h"

void
new_admin(struct ssemd * newAdmin, int adminSocket, int BUFFSIZE, char * adminAddr){
    newAdmin->fd = adminSocket;

    buffer * readBuffer = (buffer*)malloc(sizeof(buffer));

    uint8_t * dataRead = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    memset(dataRead, 0, sizeof(uint8_t) * BUFFSIZE);
    buffer_init(readBuffer, BUFFSIZE, dataRead);

    newAdmin->bufferRead = readBuffer;

    buffer * writeBuffer = (buffer*)malloc(sizeof(buffer));
    uint8_t * dataWrite = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    memset(dataWrite, 0, sizeof(uint8_t) * BUFFSIZE);
    buffer_init(writeBuffer, BUFFSIZE, dataWrite);

    newAdmin->bufferWrite = writeBuffer;

    newAdmin->isAvailable = false;

    newAdmin->adminAddr = adminAddr;
}

void removeAdmin(struct ssemd * admin) {
    close( admin->fd );

    admin->fd = 0;
    admin->isAvailable = true;
    free(admin->bufferRead->data);
    free(admin->bufferWrite->data);
}