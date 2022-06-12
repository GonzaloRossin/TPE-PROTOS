#ifndef ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8
#define ARGS_H_kFlmYm1tW9p5npzDr2opQJ9jM8

#include <stdbool.h>

#define MAX_USERS 10

struct users {
    char *name;
    char *pass;
};

// struct doh {
//     char           *host;
//     char           *ip;
//     unsigned short  port;
//     char           *path;
//     char           *query;
// };

struct socks5args {
    char           *socks_addr;
    unsigned short  socks_port;

    char *          mng_addr;
    unsigned short  mng_port;

    bool            disectors_enabled;

    //struct doh      doh;
    struct users    users[MAX_USERS];
};

struct ssemd_args {
    char           *mng_addr;
    char           *mng_port;

    char            type;
    char            code;
    char            size;
    char           *data;
};

/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void 
parse_args(const int argc, char **argv, struct socks5args *args);

void 
parse_ssemd_args(const int argc, char **argv, struct ssemd_args *args);

//helper for parse_ssemd_args
void
handleRepeatedCMD(struct ssemd_args *args, char newCode);

#endif

