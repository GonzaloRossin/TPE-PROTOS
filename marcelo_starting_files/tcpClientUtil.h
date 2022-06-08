#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>
#include "selector.h"

// Create and connect a new TCP client socket
int tcpClientSocket(const char *server, const char *service, struct selector_key * key, const fd_handler * socksv5);

#endif 