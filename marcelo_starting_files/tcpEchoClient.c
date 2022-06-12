#include "./include/tcpClientUtil.h"

#define BUFFSIZE 4096

void handleSend(struct ssemd_args *args, int sock);
void handleSend2(int sock);
void handleRecv(int sock, struct buffer * Buffer);

int main(int argc, char *argv[]) {

	struct ssemd_args * args = (struct ssemd_args *)malloc(sizeof(struct ssemd_args));
	parse_ssemd_args(argc, argv, args);

	// Create a reliable, stream socket using TCP
	int sock = tcpClientSocket(args->mng_addr,  args->mng_port); //esta funcion la usa otra persona?
	if (sock < 0) {
		print_log(FATAL, "socket() failed");
		exit(1);
	}

	//start buffer, only 1 because it sends and then recieves and then frees. no need for 2 buffers
    buffer * Buffer = (buffer*)malloc(sizeof(buffer));
    uint8_t * dataClient = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
    memset(dataClient, 0, sizeof(uint8_t) * BUFFSIZE);
    buffer_init(Buffer, BUFFSIZE, dataClient);


	// Send the HELLO to the server
	buffer_write(Buffer, 0x05);
	buffer_write(Buffer, 0x02);
	buffer_write(Buffer, 0x00);
	buffer_write(Buffer, 0x01);

	size_t bytesToSend = Buffer->write - Buffer->read;
	print_log(INFO, "Trying to send %zu bytes to socket %d\n", bytesToSend, sock);
	ssize_t bytesSent = send(sock, Buffer->data, bytesToSend, 0); //write HELLO
	buffer_read_adv(Buffer, bytesSent);
	print_log(INFO, "Sent %zu bytes: ", bytesSent);


	ssize_t bytesRecieved = 0;
	print_log(INFO, "Received: ");
	while (true) { //read HELLO
		bytesRecieved = recv(sock, Buffer->data, BUFFSIZE, 0);
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
	print_log(INFO, "done with hello handshake, starting to send a basic GET X'01'");

	handleSend(args, sock);

	while (true) { //read REQUEST
		bytesRecieved = recv(sock, Buffer->data, BUFFSIZE, 0);
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

	handleSend2(sock);

	handleRecv(sock, Buffer);

	close(sock);
	free(Buffer->data);
	free(Buffer);
	free(args);
	return 0;
}

void handleSend(struct ssemd_args *args, int sock){
	uint8_t manualSend_Request[3];
    manualSend_Request[0] = args->type; //GET
	manualSend_Request[1] = args->code; //Historic amount of connections
	manualSend_Request[2] = args->size; //Must be 00
	//manualSend_Request[3] = args->data; //Must be 00

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
}

void handleSend2(int sock){
	uint8_t hardcode_request[73];
	hardcode_request[0] = 0x47;
	int bytesSent = send(sock, hardcode_request, sizeof(uint8_t) * 71, 0);
}

void handleRecv(int sock, struct buffer * Buffer){
	while (true) { //read REQUEST
		size_t bytesRecieved = recv(sock, Buffer->data, BUFFSIZE, 0);
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
			print_log(ERROR, "close on point");
			break;
		}
	}
}