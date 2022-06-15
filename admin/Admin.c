#include "./include/adminUtil.h"

#define BUFFSIZE 4096

void handleHello(struct buffer * Buffer, int sock);
void readHello(struct buffer * Buffer, int sock);
void handleSend(struct ssemd_args *args, int sock, struct buffer * Buffer, size_t bytesToSend);
void handleRecv(int sock, struct buffer * Buffer);

int main(int argc, char *argv[]) {

	struct ssemd_args * args = (struct ssemd_args *)calloc(1, sizeof(struct ssemd_args));
	parse_ssemd_args(argc, argv, args);

	size_t bytesToSend = getSize(args);
	print_log(DEBUG, "%d", bytesToSend);

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
	handleSend(args, sock, Buffer, bytesToSend);
	// handleRecv(sock, Buffer);

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

void handleSend(struct ssemd_args *args, int sock, struct buffer * Buffer, size_t bytesToSend){
	uint8_t message[bytesToSend];
	int i=0;
    message[i++] = 0x01; //VER
	int n=0;
	while(args->admin_token[n] != 0x00){
		message[i++] = args->admin_token[n++];
	}
	message[i++] = 0x00; //TOKEN
	message[i++] = args->type; //TYPE
	message[i++] = args->cmd; //CMD
	message[i++] = args->size1;
	message[i++] = args->size2; //SIZE
	int size = 0;
	size+=args->size2; 
	if(args->size1 != 0x00){
		size+=args->size1 + 255;
	} //size1 size2 bytes for data

	n=0;
	while(n<size){
		message[i++] = args->data[n++];
	}
	if(i != bytesToSend){
		print_log(ERROR, "error crafitng admin message");
		exit(1);
	}
	print_log(DEBUG, "sending message: ");
	for(n=0; n<bytesToSend; n++){
		print_log(DEBUG, "%x ", message[n]);
	}
	// exit(1);

	// // size_t bytesSent = 0;
	// // while (bytesSent < bytesToSend){
	// // 	/
	// // }

	int bytesSent = send(sock, message, sizeof(message), 0);
	
	// print_log(INFO, "reading request");
	// while (true) { //read REQUEST
	// 	ssize_t bytesRecieved = recv(sock, Buffer->data, BUFFSIZE, 0);
	// 	if (bytesRecieved < 0) {
	// 		print_log(ERROR, "recv() failed");
	// 	}
	// 	else if (bytesRecieved == 0)
	// 		print_log(ERROR, "recv() connection closed prematurely");
	// 	else {
	// 		for(int i=0; i<bytesRecieved; i++){
	// 			print_log(INFO, "%02x ", Buffer->data[i]);
	// 		}
	// 	}
	// 	if(bytesRecieved == 10){
	// 		break;
	// 	}
	// }
	// print_log(INFO, "done with request read");
	// print_log(INFO, "sending basic get to recieve a reply from remote server");
	// //uint8_t hardcode_request[73];
	// //hardcode_request[0] = 0x47;
	// bytesSent = send(sock, hardcode_request, sizeof(uint8_t), 0);
	// print_log(INFO, "up to here its temporary\n");
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