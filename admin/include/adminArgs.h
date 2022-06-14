#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>


struct ssemd_args {
    char           *mng_addr;
    char           *mng_port;

    char            type;
    char            code;
    char            size;
    char           *data;
};


void 
parse_ssemd_args(const int argc, char **argv, struct ssemd_args *args);

//helper for parse_ssemd_args
void
handleRepeatedCMD(struct ssemd_args *args, char newCode);