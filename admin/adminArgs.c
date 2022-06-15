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
    // if(argc > 99){
    //     fprintf(stderr, "Too many arguments, usage example: -G1 -t qwertyuiop\n");
    //     exit(1);
    // } else if(argc < 3){
    //     fprintf(stderr, "Too few arguments, usage example: -G1 -t qwertyuiop\n");
    //     exit(1);
    // }
    // memset(args, 0, sizeof(struct ssemd_args));


    char           *admin_token;
    char            type;
    char            cmd;

    args->mng_addr   = "127.0.0.1";
    args->mng_port   = "8889";

    args->admin_token = NULL;
    args->type        = 0x00;
    args->cmd         = 0x00;
    args->size        = 0x00;
    args->data        = 0x00;
    
    int c;

    while (true) {
        c = getopt (argc, argv, "t:G:E:");

        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'G':
                handleRepeatedTYPE(args, 0x01);
                args->cmd = *optarg; 
                break;
            case 'E':
                handleRepeatedTYPE(args, 0x02);
                args->cmd = *optarg; 
                break;
            case 't':
                args->admin_token = optarg;
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
    if(args->type == 0 || args->cmd == 0){
        fprintf(stderr, "argument required: type. \nusage: -G for Get or -E for Edit\n");
        exit(1);
    }
    if(args->admin_token == NULL){
        fprintf(stderr, "argument required: admin token. \nusage: -t xxx\n");
        exit(1);
    }
}

void handleRepeatedTYPE(struct ssemd_args *args, char newType){
    if(args->type == 0){
        args->type = newType;
    } else {
        fprintf(stderr, "argument not accepted: %x, usage example: -G1\n", newType);
        exit(1);
    }          
}