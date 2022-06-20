#ifndef SOCKET_CREATION_H_
#define SOCKET_CREATION_H_

#include "args.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <sys/time.h>
#include <sys/signal.h>

int create_master_sockets(int * master_socket, int * master_socket_6);

#endif