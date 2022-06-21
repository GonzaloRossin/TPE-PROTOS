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
#include <sys/time.h>
#include <sys/signal.h>
#include "logger.h"

#define MAX_PENDING_CONNECTIONS   10 

int create_master_sockets(int * master_socket, int * master_socket_6, struct socks5args * args);
int create_master_socket_4(struct sockaddr_in * addr);
int create_master_socket_6(struct sockaddr_in6 * addr);

void create_admin_sockets(int *management_socket_4, int *management_socket_6, struct socks5args * args);
#endif