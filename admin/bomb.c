#include "./include/bombUtil.h"

#define BUFFSIZE 4096

// void handleAdminSend(struct bomb_args *args, int sock, struct buffer * Buffer, ssize_t bytesToSend, struct admin_parser * adminParser);
// void handleRecv(int sock, struct buffer * Buffer, struct admin_parser * adminParser);
// void parseResponse(struct buffer * Buffer);
// void processParser(struct admin_parser * adminParser);


int main(int argc, char *argv[]) {
    struct bomb_args * args = (struct bomb_args *)calloc(1, sizeof(struct bomb_args));
	parse_bomb_args(argc, argv, args);

    unsigned long currentCrack = 0;
    int bytesToSend = 7;
    buffer * Buffer = (buffer*)malloc(sizeof(buffer));
    uint8_t * dataClient = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    memset(dataClient, 0, sizeof(uint8_t) * BUFFSIZE);
    buffer_init(Buffer, BUFFSIZE, dataClient);

    //create 1000 connections
    int connections[1000] = {0};
    int i;
    print_log(INFO, "Hello :D\n");
    if(args->connections){
        for(i=0 ; i<100 ; i++){
            printf("bomb: %d\n", i);
            connections[i] = tcpBombSocket("127.0.0.1", "8080"); //(args->addr, args->port);
            if(args->crack && connections[i] != -1){
                uint8_t message[bytesToSend];
                int i=0;
                message[i++] = 0x01; //VER
                int n=0;
                // char token;
                message[i++] = 0x61; // TOKEN
                // sprintf(token, "%x", currentCrack++);
                printf("%x", currentCrack++);

                // while(args->admin_token[n] != 0x00){
                //     message[i++] = args->admin_token[n++];
                // }
                message[i++] = 0x00; // END TOKEN
                message[i++] = 0x01; //TYPE
                message[i++] = 0x01; //CMD
                message[i++] = 0x00;
                message[i++] = 0x00; //SIZE
                print_log(INFO, "perra");

                int bytesSent = send(connections[i], message, sizeof(message), MSG_CONFIRM);
                print_log(DEBUG, "sent: %d", bytesSent);
                if(bytesSent < bytesToSend){
		            print_log(ERROR, "ERROR, COULDN'T SEND ALL BYTES");
	            } else {
                    ssize_t bytesRecieved = recv(connections[i], Buffer->data, BUFFSIZE, 0);
                    if(Buffer->data[0] == 0xAA){
                        print_log(INFO, "found token");
                        exit(1);
                    }
                }
            }
        }
        exit(0);
    }

    // if(args->crack){

    // }


    // if(args->isAdmin){
    //     int sock = tcpBombSocket(args->addr,  args->port);
    //     ssize_t bytesToSend = getBombSize(args);
    //     buffer * Buffer = (buffer*)malloc(sizeof(buffer));
    //     uint8_t * dataClient = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    //     memset(dataClient, 0, sizeof(uint8_t) * BUFFSIZE);
    //     buffer_init(Buffer, BUFFSIZE, dataClient);
    //     // handleAdminSend(args, sock, Buffer, bytesToSend, adminParser);
    // }

	//send command to server
	// handleSend(args, sock, Buffer, bytesToSend, adminParser);

	return 0;
}