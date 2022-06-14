#include "./include/adminArgs.h"

static void
usage(const char *progname) {
    fprintf(stderr,
        "Usage: %s [OPTION]...\n"
        "\n"
        "   -h               Imprime la ayuda y termina.\n"
        "   -L <conf  addr>  Dirección donde se encuentra el servidor.\n"
        "   -p <SOCKS port>  Puerto donde se encuentra el servidor.\n"
        "   -v               Imprime información sobre la versión versión y termina.\n"
        "\n",
        progname);
    exit(1);
}

void 
parse_ssemd_args(const int argc, char **argv, struct ssemd_args *args) {
    if(argc > 99){
        fprintf(stderr, "Too many arguments, usage example: -G1 -t qwertyuiop\n");
        exit(1);
    } else if(argc < 3){
        fprintf(stderr, "Too few arguments, usage example: -G1 -t qwertyuiop\n");
        exit(1);
    }
    memset(args, 0, sizeof(*args)); // sobre todo para setear en null los punteros de users
    
    args->mng_addr   = "127.0.0.1";
    args->mng_port   = "8889";
    args->size       = 0x00;
    args->data       = 0x00;
    
    int c;

    while (true) {
        c = getopt (argc, argv, "G:E:t:");

        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'G':
                if(args->type == 0){
                    args->type = 0x01;
                    args->code = *optarg;
                } else {
                    fprintf(stderr, "argument not accepted: %c, usage example: -G1\n", c);
                    exit(1);
                }
 
                break;
            case 'E':
                if(args->type == 0){
                    args->type = 0x02;
                    args->code = *optarg;
                } else {
                    fprintf(stderr, "argument not accepted: %c, usage example: -G1\n", c);
                    exit(1);
                }
                break;
            case 't':
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
    if(args->type == 0 || args->code == 0){
        fprintf(stderr, "wrong usage, example: -G1\n");
        exit(1);
    }
}

void handleRepeatedCMD(struct ssemd_args *args, char newCode){
    if(args->code == 0){
        args->code = newCode;
    } else {
        fprintf(stderr, "argument not accepted: %x, usage example: -G1\n", newCode);
        exit(1);
    }          
}