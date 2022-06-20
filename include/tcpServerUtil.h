#ifndef TCPSERVERUTIL_H_
#define TCPSERVERUTIL_H_

#include <stdio.h>
#include <sys/socket.h>


// Create, bind, and listen a new TCP server socket
int setupTCPServerSocket(const char *service, const int family);

// Accept a new TCP connection on a server socket
int acceptTCPConnection(int servSock, char * clientAddr);

// Handle new TCP client
void handleTCPEchoClient(int clntSocket);

#endif 
