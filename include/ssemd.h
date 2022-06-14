#ifndef SSEMD_H_
#define SSEMD_H_

#include "buffer.h"
#include "hello.h"

enum ssemd_state {
    SSEMD_HELLO_READ_STATE = 0,
    SSEMD_HELLO_WRITE_STATE,
    SSEMD_REQUEST_READ_STATE,
    SSEMD_REQUEST_PROCESS_STATE,
    SSEMD_REQUEST_WRITE_STATE,
    SSEMD_CLOSE_STATE,
    SSEMD_ERROR_STATE,
};

struct ssemd_hello
{
    hello_parser * pr;
    buffer * w;
    buffer * r;
};

struct ssemd_connection_state {
    int init;
    enum ssemd_state ssemd_state;
    void (*on_departure) (struct ssemd * currAdmin);
    void (*on_arrival) (struct ssemd * currAdmin);
};

struct ssemd
{
    int fd;

    buffer * bufferRead;
    buffer * bufferWrite;

    struct ssemd_connection_state connection_state;

    union {
        struct ssemd_hello hello;
    } admin;

    bool isAvailable;
};

void
new_admin(struct ssemd * newAdmin, int adminSocket, int BUFFSIZE);

void removeAdmin(struct ssemd * admin);

#endif