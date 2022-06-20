#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../../include/logger.h"
#include "../../include/buffer.h"

#define MAX_ADDR_BUFFER 4096

struct bomb_args {
    char           *addr;
    char           *port;

    char           *admin_token;
    char            type;
    char            cmd;
    uint8_t         size1;
    uint8_t         size2;
    char           *data;
    bool            isAdmin;
    bool            connections;
};

void parse_bomb_args(const int argc, char **argv, struct bomb_args *args);
void handleRepeatedTYPE(struct bomb_args *args, char newType);
void handleRepeatedCMD(struct bomb_args *args, char newCMD);
void setSize(struct bomb_args * args);
ssize_t getBombSize(struct bomb_args *args);

void parseData(struct bomb_args * args);

int tcpBombSocket(const char *host, const char *service);



