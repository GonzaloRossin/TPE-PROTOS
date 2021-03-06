#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "../include/logger.h"
#include "../include/util.h"
#define TRUE   1
#define FALSE  0

#define MAXPENDING 5 // Maximum outstanding connection requests
#define BUFSIZE 256
#define MAX_ADDR_BUFFER 128

static char addrBuffer[MAX_ADDR_BUFFER];
/*
 ** Se encarga de resolver el número de puerto para service (puede ser un string con el numero o el nombre del servicio)
 ** y crear el socket pasivo, para que escuche en cualquier IP, ya sea v4 o v6
 */
int setupTCPServerSocket(const char *service, const int family, char * bindAddr) {
	int opt = TRUE;
	// Construct the server address structure
	struct addrinfo addrCriteria;                   // Criteria for address match
	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
	addrCriteria.ai_family = family;             // Any address family
	addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
	addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
	addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

	struct addrinfo *servAddr; 			// List of server addresses
	int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
	if (rtnVal != 0) {
		print_log(FATAL, "getaddrinfo() failed %s", gai_strerror(rtnVal));
		return -1;
	}

	int servSock = -1;
	// Intentamos ponernos a escuchar en alguno de los puertos asociados al servicio, sin especificar una IP en particular
	// Iteramos y hacemos el bind por alguna de ellas, la primera que funcione, ya sea la general para IPv4 (0.0.0.0) o IPv6 (::/0) .
	// Con esta implementación estaremos escuchando o bien en IPv4 o en IPv6, pero no en ambas
	for (struct addrinfo *addr = servAddr; addr != NULL && servSock == -1; addr = addr->ai_next) {
		errno = 0;
		// Create a TCP socket
		servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (servSock < 0) {
			print_log(DEBUG, "Cant't create socket on %s : %s ", printAddressPort(addr, addrBuffer), strerror(errno));  
			continue;       // Socket creation failed; try next address
		}
		if (family == AF_INET6) {
			if( setsockopt(servSock, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&opt, sizeof(opt)) < 0 )
			{
				print_log(ERROR, "set IPv6 socket options failed %s", strerror(errno));
			}
		}

		setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));
		// bind the socket to localhost port 8888
		if (bind(servSock, addr->ai_addr, addr->ai_addrlen) < 0) 
		{
			print_log(DEBUG, "Cant't bind %s", strerror(errno));  
			close(servSock);  // Close and try with the next one
			servSock = -1;
		}
		else {
			if (listen(servSock, MAXPENDING) < 0)
			{
				print_log(ERROR, "listen on IPv4 socket failes");
				close(servSock);
			} else {
				// Print local address of socket
				struct sockaddr_storage localAddr;
				socklen_t addrSize = sizeof(localAddr);
				if (getsockname(servSock, (struct sockaddr *) &localAddr, &addrSize) >= 0) {
					printSocketAddress((struct sockaddr *) &localAddr, addrBuffer);
					print_log(INFO, "Binding to %s", addrBuffer);
				}
			}
		}
	}

	freeaddrinfo(servAddr);

	return servSock;
}

int acceptTCPConnection(int servSock, char * clientAddr) {
	struct sockaddr_storage clntAddr; // Client address
	// Set length of client address structure (in-out parameter)
	socklen_t clntAddrLen = sizeof(clntAddr);

	// Wait for a client to connect
	int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
	if (clntSock < 0) {
		print_log(ERROR, "accept() failed");
		return -1;
	}

	// clntSock is connected to a client!
	printSocketAddress((struct sockaddr *) &clntAddr, addrBuffer);
	strcpy(clientAddr, addrBuffer);

	return clntSock;
}