#ifndef TCPCLIENT_H_
#define TCPCLIENT_H_

#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <getopt.h>
#include <stdbool.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "adminArgs.h"
#include "../../include/logger.h"
#include "../../include/buffer.h"
#include "../../include/util.h"
#include "parseUtil.h"
#include <math.h>


// Create and connect a new TCP client socket
int tcpClientSocket(const char *server, const char *service);

size_t getSize(struct ssemd_args *args);

#endif 