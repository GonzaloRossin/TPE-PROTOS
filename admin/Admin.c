#include "./include/adminUtil.h"

#define BUFFSIZE 4096

void handleHello(struct buffer * Buffer, int sock);
void readHello(struct buffer * Buffer, int sock);
void handleSend(struct ssemd_args *args, int sock, struct buffer * Buffer);
void handleRecv(int sock, struct buffer * Buffer);

int main(int argc, char *argv[]) {

	struct ssemd_args * args = (struct ssemd_args *)calloc(1, sizeof(struct ssemd_args));
	parse_ssemd_args(argc, argv, args);

	// Create a reliable, stream socket using TCP
	int sock = tcpClientSocket(args->mng_addr,  args->mng_port);
	if (sock < 0) {
		print_log(FATAL, "socket() failed");
		exit(1);
	}

	//start buffer, only 1 because it sends and then recieves and then frees. no need for 2 buffers
    buffer * Buffer = (buffer*)malloc(sizeof(buffer));
    uint8_t * dataClient = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    memset(dataClient, 0, sizeof(uint8_t) * BUFFSIZE);
    buffer_init(Buffer, BUFFSIZE, dataClient);


	// // Send the HELLO to the server
	// handleHello(Buffer, sock);
	// readHello(Buffer, sock);
	// print_log(INFO, "\ndone with hello handshake\n");

	// //send command to server
	// handleSend(args, sock, Buffer);
	// handleRecv(sock, Buffer);

	// size_t bytesToSend = args;

	close(sock);
	free(Buffer->data);
	free(Buffer);
	free(args);
	return 0;
}

void handleHello(struct buffer * Buffer, int sock){
	buffer_write(Buffer, 0x05);
	buffer_write(Buffer, 0x02);
	buffer_write(Buffer, 0x00);
	buffer_write(Buffer, 0x01);

	size_t bytesToSend = Buffer->write - Buffer->read;
	print_log(INFO, "Trying to send %zu bytes to socket %d\n", bytesToSend, sock);
	ssize_t bytesSent = send(sock, Buffer->data, bytesToSend, 0); //write HELLO
	if(bytesSent>0){
		buffer_read_adv(Buffer, bytesSent);
		print_log(INFO, "Sent %zu bytes: ", bytesSent);
	} else {
		print_log(ERROR, "Error sending to sock %d", sock);
		exit(1);
	}
}

void readHello(struct buffer * Buffer, int sock){
	ssize_t bytesRecieved = 0;
	print_log(INFO, "Received: ");
	while (true) { //read HELLO
		bytesRecieved += recv(sock, Buffer->data, BUFFSIZE, 0);
		if (bytesRecieved < 0) {
			print_log(ERROR, "recv() failed");
		}
		else if (bytesRecieved == 0)
			print_log(ERROR, "recv() connection closed prematurely");
		else {
			for(int i=0; i<bytesRecieved; i++){
				print_log(INFO, "%02x ", Buffer->data[i]);
			}
		}
		if(bytesRecieved == 2){
			break;
		}
	}
}

void handleSend(struct ssemd_args *args, int sock, struct buffer * Buffer){
	uint8_t manualSend_Request[3];
    manualSend_Request[0] = args->type; //GET
	manualSend_Request[1] = args->cmd; //Historic amount of connections
	manualSend_Request[2] = args->size1; //Must be 00
	//manualSend_Request[3] = args->data; //Must be 00

	print_log(INFO, "sending socks request, from here its temporary");
	uint8_t hardcode_request[14];
	hardcode_request[0] = 0x05;
	hardcode_request[1] = 0x01;
	hardcode_request[2] = 0x00;
	hardcode_request[3] = 0x03;
	hardcode_request[4] = 0x07;
	hardcode_request[5] = 0x77;
	hardcode_request[6] = 0x74;
	hardcode_request[7] = 0x74;
	hardcode_request[8] = 0x72;
	hardcode_request[9] = 0x2E;
	hardcode_request[10] = 0x69;
	hardcode_request[11] = 0x6E;
	hardcode_request[12] = 0x00;
	hardcode_request[13] = 0x50;

	int bytesSent = send(sock, hardcode_request, sizeof(hardcode_request), 0);
	
	print_log(INFO, "reading request");
	while (true) { //read REQUEST
		ssize_t bytesRecieved = recv(sock, Buffer->data, BUFFSIZE, 0);
		if (bytesRecieved < 0) {
			print_log(ERROR, "recv() failed");
		}
		else if (bytesRecieved == 0)
			print_log(ERROR, "recv() connection closed prematurely");
		else {
			for(int i=0; i<bytesRecieved; i++){
				print_log(INFO, "%02x ", Buffer->data[i]);
			}
		}
		if(bytesRecieved == 10){
			break;
		}
	}
	print_log(INFO, "done with request read");
	print_log(INFO, "sending basic get to recieve a reply from remote server");
	//uint8_t hardcode_request[73];
	//hardcode_request[0] = 0x47;
	bytesSent = send(sock, hardcode_request, sizeof(uint8_t), 0);
	print_log(INFO, "up to here its temporary\n");
}

void handleRecv(int sock, struct buffer * Buffer){
	print_log(INFO, "Reading server reply, in the future this will be server response to admin client request:\n");
	while (true) { //read REQUEST
		ssize_t bytesRecieved = recv(sock, Buffer->data, BUFFSIZE, 0);
		if (bytesRecieved < 0) {
			print_log(ERROR, "recv() failed");
		} else if (bytesRecieved == 0) {
			print_log(ERROR, "recv() connection closed prematurely");
			break;
		} else {
			for(int i=0; i<bytesRecieved; i++){
				print_log(INFO, "%s ", Buffer->data);
				break;
			}
		}
		if(bytesRecieved == 325){
			print_log(INFO, "close on point");
			break;
		}
	}
}