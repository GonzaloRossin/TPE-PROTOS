#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8

#include <stdbool.h>
#include <sys/types.h>
#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <stdint.h>
#include <netinet/in.h>     // AF_UNSPEC
#include <arpa/inet.h>      // inet_pton
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define MAX_USERS 10

struct users {
    char *name;
    char *pass;
};

struct socks5args {
    char           *socks_addr;
    char           *socks_addr6;
    int             socks_family;
    unsigned short  socks_port;
    struct sockaddr_in  socks_addr_info;
    struct sockaddr_in6 socks_addr_info6;

    char *          mng_addr;
    char *          mng_addr6;
    int             mng_family;
    unsigned short  mng_port;
    struct sockaddr_in  mng_addr_info;
    struct sockaddr_in6 mng_addr_info6;

    bool            disectors_enabled;

    char *          admin_token;

    struct users    users[MAX_USERS];
};

/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void 
parse_args(const int argc, char **argv, struct socks5args *args);

int
get_addr_type(char *address);

void
address(char *address, int port, struct sockaddr_in *addr);

void
address6(char *address, int port, struct sockaddr_in6 *addr);

#endif

