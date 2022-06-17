#include "./include/adminUtil.h"

#define BUFFSIZE 4096

void handleSend(struct ssemd_args *args, int sock, struct buffer * Buffer, size_t bytesToSend);
void handleRecv(int sock, struct buffer * Buffer);
void parseResponse(struct buffer * Buffer);

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
	print_log(DEBUG, "%0x %0x %0x %0x", Buffer->data[0], Buffer->data[1], Buffer->data[2], Buffer->data[3]);



	//send command to server
	handleSend(args, sock, Buffer, bytesToSend);

	memset(dataClient, 0, sizeof(uint8_t) * BUFFSIZE);

	//read reply
	handleRecv(sock, Buffer);

	close(sock);
	free(Buffer->data);
	free(Buffer);
	free(args);
	return 0;
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

	n = 0;
	while(n<size){
		message[i++] = args->data[n++];
	}
	if(i != bytesToSend){
		print_log(ERROR, "error crafting admin message");
		exit(1);
	}
	print_log(DEBUG, "sending message: ");
	for(n=0; n<bytesToSend; n++){
		print_log(DEBUG, "%d: %x ",n, message[n]);
	}

	int bytesSent = send(sock, message, sizeof(message), 0);
	// // size_t bytesSent = 0;
	// // while (bytesSent < bytesToSend){
	// // 	/
	// // }

	if(bytesSent < bytesToSend){
		print_log(ERROR, "ERROR, COULDN'T SEND ALL BYTES");
	}
}

void handleRecv(int sock, struct buffer * Buffer){
	print_log(INFO, "Reading server reply\n");
	while (true) { //read REQUEST
		ssize_t bytesRecieved = recv(sock, Buffer->data, BUFFSIZE, 0);
		if (bytesRecieved < 0) {
			print_log(ERROR, "recv() failed");
		} else if (bytesRecieved == 0) {
			print_log(ERROR, "recv() connection closed prematurely");
			break;
		} else {
			print_log(DEBUG, "%0x %0x %0x %0x", Buffer->data[4], Buffer->data[5], Buffer->data[6], Buffer->data[7]);
			parseResponse(Buffer);

			break;
		}
	}
}

void parseResponse(struct buffer * Buffer){
	int n = 0;
	struct admin_parser * adminParser = (struct admin_parser *)malloc(sizeof(struct admin_parser));
	int j;
	unsigned int number;
	double power; 
	adminParser->number = 0;
	adminParser->size = 0;
	adminParser->state = read_status;
	while(adminParser->state != read_close){
		switch (adminParser->state){
			case read_status:
				if(Buffer->data[n] == SSEMD_RESPONSE){
					adminParser->state = read_response_code;
				} else if(Buffer->data[n] == SSEMD_ERROR){
					print_log(INFO, "The server has replied the following error:\n");
					adminParser->state = read_error_code;
				} else {
					print_log(INFO, "You have recieved a message with unknown STATUS\n");
					adminParser->state = read_error;
				}
				n++;
				break;

			case read_response_code:
				if(Buffer->data[n] == SSEMD_RESPONSE_OK){
					print_log(INFO, "OK, The requested command was processed succesfully\n");
					adminParser->code = SSEMD_RESPONSE_OK;
					adminParser->state = read_done;
				} else if(Buffer->data[n] == SSEMD_RESPONSE_LIST){
					print_log(INFO, "The list of users:passwords is:\n");
					adminParser->code = SSEMD_RESPONSE_LIST;
					adminParser->state = read_size1;
				} else if(Buffer->data[n] == SSEMD_RESPONSE_INT){
					print_log(INFO, "Response number:\n");
					adminParser->code = SSEMD_RESPONSE_INT;
					adminParser->state = read_size1;
				} else if(Buffer->data[n] == SSEMD_RESPONSE_BOOL){
					print_log(INFO, "Boolean answer:\n");
					adminParser->code = SSEMD_RESPONSE_BOOL;
					adminParser->state = read_size1;
				} else {
					print_log(INFO, "You have recieved a message with unknown CODE\n");
					adminParser->state = read_error;
				}
				n++;
				break;

			case read_size1:
				adminParser->size = 0;
				if(Buffer->data[n] != 0x00){
					adminParser->size += 256 * (uint8_t) Buffer->data[n];
				}
				adminParser->state = read_size2;
				n++;
				break;
			
			case read_size2:
				if(Buffer->data[n] == 0x00){
					if(adminParser->size == 0){
						print_log(INFO, "You recieved a message that should contain DATA but it doesn't\n");
						adminParser->state = read_error;
					} else {
						adminParser->state = prepare_data;
					}
				} else {
					adminParser->size += (uint8_t) Buffer->data[n];
					adminParser->state = prepare_data;
				}
				n++;
				break;

			case prepare_data:
				adminParser->data = (unsigned char *)malloc(sizeof(uint8_t) * adminParser->size);
				adminParser->dataPointer = 0;
				adminParser->state = read_data;
				break;

			case read_data:
				adminParser->data[adminParser->dataPointer++] = Buffer->data[n++];
				if(adminParser->dataPointer == adminParser->size){
					adminParser->state = read_done;
				}
				break;

			case read_done:
				if(adminParser->size > 0) {
					int i;
					for(i=0; i<adminParser->size; i++){
						switch (adminParser->code)
						{
						case SSEMD_RESPONSE_OK:
							print_log(ERROR, "This response shouldn't have any DATA, error");
							adminParser->state = read_error;
							break;
						
						case SSEMD_RESPONSE_LIST:
							printf("%c", adminParser->data[i]);
							if(adminParser->data[i] == '\0'){
								printf("\n");
							}
							break;
						
						case SSEMD_RESPONSE_INT:
								j = i;					
								for(j; j<i+4; j++){
									power = pow(256, 4 - j -1);
									number = adminParser->data[j];// - '0';
									// letter = adminParser->data[j] + '0';
									printf("%0x ", adminParser->data[j]);
									adminParser->number +=  number * power;
								}

							printf("\n%u", adminParser->number);
							adminParser->state = read_close;
							i+=4;
							break;

						case SSEMD_RESPONSE_BOOL:
							if(adminParser->data[i] == 0x00){
								printf("TRUE\n");

							} else if(adminParser->data[i] == 0x11){
								printf("FALSE\n");
							}else{
								print_log(ERROR, "Wrong response for a BOOL\n");
								adminParser->state = read_error;
							}
							break;

						default:
							print_log(ERROR, "Wrong CODE detected\n");
							adminParser->state = read_error;

							break;
						}
					}
					adminParser->state = read_close;
					free(adminParser->data);
				} else {
					adminParser->state = read_close;
				}
				printf("\n");
				break;

			case read_error:
				print_log(INFO, "exiting because of error\n");
				adminParser->state = read_close;
				break;

			default:
				print_log(ERROR, "Unknown parser state\n");
				adminParser->state = read_error;
				break;
		}
	}
	free(adminParser);
}