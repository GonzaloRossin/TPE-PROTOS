#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>
#include "../include/args.h"

static unsigned short
port(const char *s) {
     char *end     = 0;
     const long sl = strtol(s, &end, 10);

     if (end == s|| '\0' != *end
        || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
        || sl < 0 || sl > USHRT_MAX) {
         fprintf(stderr, "port should in in the range of 1-65536: %s\n", s);
         exit(1);
         return 1;
     }
     return (unsigned short)sl;
}

static void
user(char *s, struct users *user) {
    char *p = strchr(s, ':');
    if(p == NULL) {
        fprintf(stderr, "password not found\n");
        exit(1);
    } else {
        *p = 0;
        p++;
        user->name = s;
        user->pass = p;
    }

}

static void
version(void) {
    fprintf(stderr, "socks5v version 0.0\n"
                    "ITBA Protocolos de Comunicación 2022/1 -- Grupo X\n"
                    "AQUI VA LA LICENCIA\n");
}

static void
usage(const char *progname) {
    fprintf(stderr,
        "Usage: %s [OPTION]...\n"
        "\n"
        "   -h               Imprime la ayuda y termina.\n"
        "   -N               Deshabilita los passwords dissectors.\n"
        "   -l <SOCKS addr>  Dirección donde servirá el proxy SOCKS.\n"
        "   -L <conf  addr>  Dirección donde servirá el servicio de management.\n"
        "   -p <SOCKS port>  Puerto entrante conexiones SOCKS.\n"
        "   -P <conf port>   Puerto entrante conexiones configuracion\n"
        "   -u <name>:<pass> Usuario y contraseña de usuario que puede usar el proxy. Hasta 10.\n"
        "   -v               Imprime información sobre la versión versión y termina.\n"

        "\n",
        progname);
    exit(1);
}

void 
parse_args(const int argc, char **argv, struct socks5args *args) {
    memset(args, 0, sizeof(*args)); // sobre todo para setear en null los punteros de users

    args->socks_addr = "0.0.0.0";
    args->socks_addr6 = "::";
    args->socks_port = 1080;
    args->socks_family = AF_UNSPEC;

    args->mng_addr   = "127.0.0.1";
    args->mng_addr6 = "::1";
    args->mng_port   = 8080;
    args->mng_family = AF_UNSPEC;

    args->disectors_enabled = true;

    int c;
    int nusers = 0;

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            { 0,           0,                 0, 0 }
        };

        c = getopt_long(argc, argv, "hl:L:Np:P:u:vt:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'l':
                args->socks_family = get_addr_type(optarg);
                if (args->socks_family == AF_INET) {
                    args->socks_addr = optarg;
                    address(args->socks_addr, args->socks_port, &args->socks_addr_info);
                } else if (args->socks_family == AF_INET6) {
                    args->socks_addr6 = optarg;
                    address6(args->socks_addr6, args->socks_port, &args->socks_addr_info6);
                }
                break;
            case 'L':
                args->mng_family = get_addr_type(optarg);
                if (args->mng_family == AF_INET) {
                    args->mng_addr = optarg;
                    address(args->mng_addr, args->mng_port, &args->mng_addr_info);
                }
                else if (args->mng_family == AF_INET6){
                    args->mng_addr6 = optarg;
                    address6(args->mng_addr6, args->mng_port, &args->mng_addr_info6);
                }
                break;
            case 'N':
                args->disectors_enabled = false;
                break;
            case 'p':
                args->socks_port = port(optarg);
                break;
            case 'P':
                args->mng_port   = port(optarg);
                break;
            case 't':
                if (strlen(optarg) > 99) {
                    fprintf(stderr, "maximun token length exceeded: %d.\n", 99);
                    exit(1);
                }
                args->admin_token = optarg;
                break;
            case 'u':
                if(nusers >= MAX_USERS) {
                    fprintf(stderr, "maximun number of command line users reached: %d.\n", MAX_USERS);
                    exit(1);
                } else {
                    user(optarg, args->users + nusers);
                    nusers++;
                }
                break;
            case 'v':
                version();
                exit(0);
                break;
            default:
                fprintf(stderr, "unknown argument %d.\n", c);
                exit(1);
        }

    }
    if (optind < argc) {
        fprintf(stderr, "argument not accepted: ");
        while (optind < argc) {
            fprintf(stderr, "%s ", argv[optind++]);
        }
        fprintf(stderr, "\n");
        exit(1);
    }
    if(args->admin_token == NULL){
        fprintf(stderr, "argument required: admin token. \nusage: -t xxx\n");
        exit(1);
    }
}

int
get_addr_type(char *address) {
    struct in_addr inaddr;
    struct in6_addr in6addr;
    int r_4, r_6;

    // Try to match with IPv4
    r_4 = inet_pton(AF_INET, address, &inaddr);

    // IPv4 unsuccessful, try with IPv6
    if (r_4 <= 0)
    {
        // Try to match with IPv4
        r_6 = inet_pton(AF_INET6, address, &in6addr);

        // IPv6 error, exit
        if (r_6 <= 0)
        {
            printf("Cannot determine address family %s, please try again with a valid address.\n", address);
            fprintf(stderr, "Address resolution error\n");
            exit(0);
        }
        else
        {
            return AF_INET6;
        }
    }
    else
    {
        return AF_INET;
    }
}

void
address(char *address, int port, struct sockaddr_in *addr)
{
    int r_4;
    // Try to match with IPv4
    r_4 = inet_pton(AF_INET, address, &addr->sin_addr);

    // IPv4 unsuccessful, try with IPv6
    if (r_4 <= 0)
    {
        printf("Cannot determine address family %s, please try again with a valid address.\n", address);
        fprintf(stderr, "Address resolution error\n");
        // free_memory();
        // exit(0);
    }
    else
    {
        addr->sin_family = AF_INET;
        addr->sin_port = htons(port);
    }
}

void
address6(char *address, int port, struct sockaddr_in6 *addr)
{
    // Try to match with IPv4
    int r_6 = inet_pton(AF_INET6, address, &addr->sin6_addr);

    // IPv6 error, exit
    if (r_6 <= 0)
    {
        printf("Cannot determine address family %s, please try again with a valid address.\n", address);
        fprintf(stderr, "Address resolution error\n");
    }
    else
    {
        addr->sin6_family = AF_INET6;
        addr->sin6_port = htons(port);
    }
}