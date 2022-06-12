#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "args.h"
#include "logger.h"
#include "buffer.h"

// Create and connect a new TCP client socket
int tcpClientSocket(const char *server, const char *service);

#endif 