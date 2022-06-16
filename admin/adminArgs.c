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
    char           *admin_token;
    char            type;
    char            cmd;

    args->mng_addr   = "127.0.0.1";
    args->mng_port   = "8889";

    args->admin_token = NULL;
    args->type        = 0x00;
    args->cmd         = 0x00;
    args->size1        = 0x00;
    args->size2        = 0x00;
    args->data        = NULL;

    
    int c;

    while (true) {
        c = getopt (argc, argv, "t:GE12345678d:");

        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 't':
                args->admin_token = optarg;
                break;
            case 'G':
                handleRepeatedTYPE(args, 0x01);
                break;
            case 'E':
                handleRepeatedTYPE(args, 0x02);
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
                handleRepeatedCMD(args, c-'0'); //ascii to int
                break;
            case 'd':
                args->data = optarg;
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
    setSize(args);
    checkRequiredParams(args);
    if(args->size1 != 0x00 && args->size2 != 0x00){
        parseData(args);
    }

}

void parseData(struct ssemd_args * args){
    char * data = args->data;
    uint8_t * toRet = (uint8_t *)args->data;
    int i = 0;
    if(args->cmd == 0x01){
        while(data[i] != '\0'){
            toRet[i] = data[i] - '0'; //ascii to int
            i++;
        }
        int a;
        for(a=0; a<i; a++){
            args->data[a] = toRet[a];
        }
        // memcpy(args->data, toRet, i);
        // args->data = (char*)toRet;
    }
}

void setSize(struct ssemd_args * args){
    if(args->data == NULL){
        args->size1 = 0x00;
        args->size2 = 0x00;
    } else {
        int i=0;
        int size = 0;
        while(args->data[i] != 0x00){
            // fprintf(stderr, "%c", (char)args->data[i]);
            size++;
            i++;
        }
        // fprintf(stderr, "\nsize:%d\n", size);
        if(size > 65656){
            fprintf(stderr, "\nsize too big:%d\n", size);
            exit(1);
        } else if(size > 255){
            args->size1 = size-255;
            args->size2 = 255-(size-255);
        } else {
            args->size1 = 0x00;
            args->size2 = size;
        }
        fprintf(stderr, "size1: %d, %c\n", args->size1, args->size1);
        fprintf(stderr, "size2: %d, %c\n", args->size2, args->size2);
    }
}

void checkRequiredParams(struct ssemd_args *args){
    if(args->type == 0 || args->cmd == 0){
        fprintf(stderr, "argument required: type. \nusage: -G for Get or -E for Edit\n");
        exit(1);
    }
    if(args->cmd == 0){
        fprintf(stderr, "argument required: command. \nusage: -G2 for Get concurrent connections\n");
        exit(1);
    }
    if(args->admin_token == NULL){
        fprintf(stderr, "argument required: admin token. \nusage: -t xxx\n");
        exit(1);
    }
    // if(args->cmd > 0x07 && args->type == 0x01){
    //     fprintf(stderr, "argument not accepted: %x, -G has max of 6\n", args->cmd); //not anymore
    //     exit(1);
    // }
}

void handleRepeatedTYPE(struct ssemd_args *args, char newType){
    if(args->type == 0){
        args->type = newType;
    } else {
        fprintf(stderr, "argument not accepted: %x, usage example: -G1\n", newType);
        exit(1);
    }          
}

void handleRepeatedCMD(struct ssemd_args *args, char newCMD){
    if(args->cmd == 0){
        args->cmd = newCMD;
    } else {
        fprintf(stderr, "argument not accepted: %x, usage example: -G1\n", newCMD);
        exit(1);
    }          
}