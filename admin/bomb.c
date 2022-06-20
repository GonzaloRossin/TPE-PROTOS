#include "./include/bombUtil.h"

#define BUFFSIZE 4096

void handleAdminSend(struct ssemd_args *args, int sock, struct buffer * Buffer, ssize_t bytesToSend, struct admin_parser * adminParser);
// void handleRecv(int sock, struct buffer * Buffer, struct admin_parser * adminParser);
// void parseResponse(struct buffer * Buffer);
// void processParser(struct admin_parser * adminParser);

int main(int argc, char *argv[]) {
    struct bomb_args * args = (struct bomb_args *)calloc(1, sizeof(struct bomb_args));
	parse_bomb_args(argc, argv, args);


    //create 1000 connections
    int connections[1000];
    int i;
    print_log(INFO, "Hello :D\n");
    if(args->connections){
        for(i=0 ; i<9 ; i++){
            printf("bomb: %d\n", i);
            connections[i] = tcpBombSocket(args->addr, args->port);
            if (connections[i] < 0) {
                print_log(FATAL, "%d failed", i);
            }
        }
        exit(0);
    }



    if(args->isAdmin){
        int sock = tcpBombSocket(args->addr,  args->port);
        ssize_t bytesToSend = getBombSize(args);
        buffer * Buffer = (buffer*)malloc(sizeof(buffer));
        uint8_t * dataClient = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
        memset(dataClient, 0, sizeof(uint8_t) * BUFFSIZE);
        buffer_init(Buffer, BUFFSIZE, dataClient);

        // handleAdminSend(args, sock, Buffer, bytesToSend, adminParser);
    }

	//send command to server
	// handleSend(args, sock, Buffer, bytesToSend, adminParser);

	return 0;
}