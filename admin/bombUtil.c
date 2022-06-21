#include "./include/bombUtil.h"


static void
usage(const char *progname) {
    fprintf(stderr,
        "Usage: %s [OPTIONS]...\n"
        " -h                prints help and exits\n"
        " -A                fake admin packet\n"
        " -P <port_number>  Set port number\n"
        " -L <addr>         Set addr\n"
        " -C                Create 1000 connections"
        "\n",
        progname);
    exit(1);
}

static void
version(const char *progname) {
    fprintf(stderr,
        "This program works as a stress tester for the rest of the project\n");
    exit(1);
}

void 
parse_bomb_args(const int argc, char **argv, struct bomb_args *args) {
    // char           *admin_token;
    // char            type;
    // char            cmd;

    args->addr   = "127.0.0.1";
    args->port   = "8080";

    args->admin_token = NULL;
    args->type        = 0x00;
    args->cmd         = 0x00;
    args->size1        = 0x00;
    args->size2        = 0x00;
    args->data        = NULL;
    args->isAdmin     = false;
    args->connections     = false;
    args->crack         = false;

    
    int c;

    while (true) {
        c = getopt (argc, argv, "t:GE12345678d:hvL:P:ACcn");

        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'A':
                args->isAdmin = true;
                break;
            case 'C':
                args->connections = true;
                break;
            case 'c':
                args->crack = true;
                break;
            case 't':
                args->admin_token = optarg;
                break;
            case 'n':
                args->number = atoi(optarg);
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
                args->addr = optarg;
                break;
            case 'P':
                args->port = optarg;
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
    if(args->size1 != 0x00 && args->size2 != 0x00){
        parseData(args);
    }

}
void parseData(struct bomb_args * args){
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
void setSize(struct bomb_args * args){
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
void handleRepeatedTYPE(struct bomb_args *args, char newType){
    if(args->type == 0){
        args->type = newType;
    } else {
        fprintf(stderr, "argument not accepted: %x, usage example: -G1\n", newType);
        exit(1);
    }          
}
void handleRepeatedCMD(struct bomb_args *args, char newCMD){
    if(args->cmd == 0){
        args->cmd = newCMD;
    } else {
        fprintf(stderr, "argument not accepted: %x, usage example: -G1\n", newCMD);
        exit(1);
    }          
}

int tcpBombSocket(const char *host, const char *service) {
	char addrBuffer[MAX_ADDR_BUFFER];
	struct addrinfo addrCriteria;                   // Criteria for address match
	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
	addrCriteria.ai_family = AF_UNSPEC;             // v4 or v6 is OK
	addrCriteria.ai_socktype = SOCK_STREAM;         // Only streaming sockets
	addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

	// Get address(es)
	struct addrinfo *servAddr; // Holder for returned list of server addrs
	int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr); //es bloqueante, ojo, hacerlo en un hijo/thread
	// int rtnVal = getaddrinfo("127.0.0.1", "8889", &addrCriteria, &servAddr);

	if (rtnVal != 0) {
		print_log(ERROR, "getaddrinfo() failed %s", gai_strerror(rtnVal));
		return -1;
	}

	int sock = -1;
	for (struct addrinfo *addr = servAddr; addr != NULL && sock == -1; addr = addr->ai_next) {
		// Create a reliable, stream socket using TCP
		sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		//setear socket como no bloqueante
		if (sock >= 0) {
			errno = 0;
			// Establish the connection to the server
			if ( connect(sock, addr->ai_addr, addr->ai_addrlen) != 0) { //se bloquea hasta que se conecte
				// print_log(INFO, "can't connectto %s: %s", printAddressPort(addr, addrBuffer), strerror(errno));
				print_log(ERROR, "cant connect");
                close(sock); 	// Socket connection failed; try next address
				sock = -1;
			}
		} else {
            print_log(ERROR, "cant create socket");
			// print_log(DEBUG, "Can't create client socket on %s",printAddressPort(addr, addrBuffer));
		}
	}

	freeaddrinfo(servAddr); 
	return sock;
}

ssize_t getBombSize(struct bomb_args *args){
	ssize_t toRet = 0;
	toRet++; //1 byte for VER
	int n=0;
	while(args->admin_token[n]!=0x00){
		n++;
	}
	toRet += n; //n bytes for TOKEN
	toRet++; //1 byte for \0 to mark end of TOKEN
	toRet++; //1 byte for TYPE
	toRet++; //1 byte for CMD
	toRet += 2; //2 bytes for SIZE
	toRet += args->size2; 
	if(args->size1 != 0x00){
		toRet+=args->size1 + 255;
	} //size1 size2 bytes for data
	return toRet;
}
