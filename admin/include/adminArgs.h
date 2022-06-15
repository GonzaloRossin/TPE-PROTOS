#ifndef ADMIN_ARGS_H
#define ADMIN_ARGS_H

#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include "adminUtil.h"


struct ssemd_args {
    char           *mng_addr;
    char           *mng_port;

    char           *admin_token;
    char            type;
    char            cmd;
    uint8_t         size1;
    uint8_t         size2;
    char           *data;
};


void 
parse_ssemd_args(const int argc, char **argv, struct ssemd_args *args);

//helpers for parse_ssemd_args
void handleRepeatedTYPE(struct ssemd_args *args, char newCode);
void handleRepeatedCMD(struct ssemd_args *args, char newCMD);
void checkRequiredParams(struct ssemd_args *args);
void setSize(struct ssemd_args * args);


#endif