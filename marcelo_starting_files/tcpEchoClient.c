#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "./include/logger.h"
#include "./include/tcpClientUtil.h"

#define BUFFSIZE 512

int main(int argc, char *argv[]) {

	if (argc != 4) {
		print_log(FATAL, "usage: %s <Server Name/Address> <Echo Word> <Server Port/Name>", argv[0]);
	}

	char *server = argv[1];     // First arg: server name IP address 
	char *echoString = argv[2]; // Second arg: string to echo

	// Third arg server port
	char * port = argv[3];

	// Create a reliable, stream socket using TCP
	int sock = tcpClientSocket(server, port);
	if (sock < 0) {
		print_log(FATAL, "socket() failed");
	}

	size_t echoStringLen = strlen(echoString); // Determine input length

	uint8_t * buffer = (uint8_t *)calloc(1, sizeof(uint8_t) * BUFFSIZE);

	// Send the HELLO to the server
	uint8_t manualSend_Hello[4];
    manualSend_Hello[0] = 0x05;
	manualSend_Hello[1] = 0x02;
	manualSend_Hello[2] = 0x00;
	manualSend_Hello[3] = 0x01;
	ssize_t numBytes = send(sock, manualSend_Hello, sizeof(manualSend_Hello), 0);
	if (numBytes < 0 || numBytes != 4)
		print_log(DEBUG, "send() failed expected %zu sent %zu", echoStringLen, numBytes);


	unsigned int totalBytesRcvd = 0; // Count of total bytes received
	print_log(INFO, "Received: ");     // Setup to print the echoed string
	while (totalBytesRcvd < 2 && numBytes >=0) { //HELLO
		numBytes = recv(sock, buffer, BUFFSIZE, 0);
		if (numBytes < 0) {
			print_log(ERROR, "recv() failed");
		}
		else if (numBytes == 0)
			print_log(ERROR, "recv() connection closed prematurely");
		else {
			totalBytesRcvd += numBytes; // Keep tally of total bytes
			for(int i=0; i<2; i++){
				print_log(INFO, "%02x ", buffer[i]);      // Print the echo buffer
			}
		}
	}
	print_log(INFO, "done with hello handshake, starting to send a basic GET X'01'");


	memset(buffer, 0, sizeof(sizeof(uint8_t) * BUFFSIZE));
	uint8_t manualSend_Request[3];
    manualSend_Request[0] = 0x01; //GET
	manualSend_Request[1] = 0x01; //Historic amount of connections
	manualSend_Request[2] = 0x00; //Must be 00
	numBytes = send(sock, manualSend_Request, sizeof(manualSend_Request), 0);
	if (numBytes < 0 || numBytes != 3)
		print_log(DEBUG, "send() failed expected %zu sent %zu", echoStringLen, numBytes);

	totalBytesRcvd = 0; // Count of total bytes received
	print_log(INFO, "Received: ");     // Setup to print the echoed string
	while (totalBytesRcvd < 5 && numBytes >=0) { //HELLO
		numBytes = recv(sock, buffer, BUFFSIZE, 0);
		if (numBytes < 0) {
			print_log(ERROR, "recv() failed");
		}
		else if (numBytes == 0)
			print_log(ERROR, "recv() connection closed prematurely");
		else {
			totalBytesRcvd += numBytes; // Keep tally of total bytes
			for(int i=0; i<5; i++){
				print_log(INFO, "%02x ", buffer[i]);      // Print the echo buffer
			}
		}
	}

	close(sock);
	free(buffer);
	return 0;
}