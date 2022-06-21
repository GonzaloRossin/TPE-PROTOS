#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../include/logger.h"
#include "../include/buffer.h"

#define BUFFSIZE 4096
#define MAX_ADDR_BUFFER 128

int tcpBombSocket(const char *host, const char *service);

int main(int argc, char *argv[]) {

    //create n connections
    int connections[1000];
    int i;
    print_log(INFO, "Hello :D\n");
    for(i=0 ; i<99 ; i++){
        printf("bomb: %d\n", i);
        connections[i] = tcpBombSocket("127.0.0.1", "1080");
        if (connections[i] < 0) {
            print_log(FATAL, "%d failed", i);
        }
    }
    print_log(INFO, "finshed, bye\n");
    exit(0);
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