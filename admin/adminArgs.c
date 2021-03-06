#include "./include/adminArgs.h"

static void
usage(const char *progname) {
    fprintf(stderr,
        "Usage: %s [OPTIONS]...\n"
        "\n"
        "   -h               Prints help and exits.\n"
        "   -t <token>       REQUIRED to specify Server Admin Token"
        "   -G or -E         For a GET or EDIT type command \n"
        "   -d               To specify DATA to send \n"
        "   -#               Number to specify which command of current type:\n"
        "       -G1                 Get historic quantity of connections\n"
        "       -G2                 Get quantity of current connections\n"
        "       -G3                 Get quantity of bytes transferred\n"
        "       -G4                 Get list of Users\n"
        "       -G5                 Get current password dissector status\n"
        "       -G6                 Get Server authentication status\n"
        "       -G7                 Get Server BUFFER SIZE\n"
        "       -G8                 Get Server timeout\n"
        "       -E1 -d <#>          Edit Client buffer size\n"
        "       -E2 -d <#>          Edit Select timeout\n"
        "       -E3                 Turn ON password dissector\n"
        "       -E4                 Turn OFF password dissector\n"
        "       -E5 -d <user:pass>  Add a User\n"
        "       -E6 -d <user:pass>  Remove a User\n"
        "       -E7                 Turn ON password authentication\n"
        "       -E8                 Turn OFF password authentication\n"
        "   -L <conf  addr>  Current Server direction.\n"
        "   -p <SOCKS port>  Current Server port.\n"
        "   -v               Prints info about version and exits\n"
        "\n",
        progname);
    exit(1);
}

static void
version(const char *progname) {
    fprintf(stderr,
        "This program currently implements SSEMD protocol Version 1.0, run with -h for more info\n");
    exit(1);
}

void 
parse_ssemd_args(const int argc, char **argv, struct ssemd_args *args) {
    // char           *admin_token;
    // char            type;
    // char            cmd;

    args->mng_addr   = "127.0.0.1";
    args->mng_port   = "8080";

    args->admin_token = NULL;
    args->type        = 0x00;
    args->cmd         = 0x00;
    args->size1        = 0x00;
    args->size2        = 0x00;
    args->data        = NULL;

    
    int c;

    while (true) {
        c = getopt (argc, argv, "t:GE12345678d:hvL:P:");

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
            case 'L':
                args->mng_addr = optarg;
                break;
            case 'P':
                args->mng_port = optarg;
                break;

            case 'v':
                version(argv[0]);
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
            size++;
            i++;
        }
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
    if(args->type == 0x02 && (args->cmd == 0x05 || args->cmd == 0x06)){ //data must be of user:pass
        if(args->data == NULL){
            fprintf(stderr, "Argument requires data of style [user:pass]\n");
            exit(1);
        }
        char *p = strchr(args->data, ':');
        if(p == NULL) {
            fprintf(stderr, "Argument requires data of style [user:pass]\n");
            exit(1);
        }
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

void handleRepeatedCMD(struct ssemd_args *args, char newCMD){
    if(args->cmd == 0){
        args->cmd = newCMD;
    } else {
        fprintf(stderr, "argument not accepted: %x, usage example: -G1\n", newCMD);
        exit(1);
    }          
}