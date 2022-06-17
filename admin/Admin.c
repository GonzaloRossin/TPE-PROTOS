#include "./include/adminUtil.h"

#define BUFFSIZE 4096

void handleSend(struct ssemd_args *args, int sock, struct buffer * Buffer, size_t bytesToSend, struct admin_parser * adminParser);
void handleRecv(int sock, struct buffer * Buffer, struct admin_parser * adminParser);
void parseResponse(struct buffer * Buffer);
void processParser(struct admin_parser * adminParser);

int main(int argc, char *argv[]) {

	struct ssemd_args * args = (struct ssemd_args *)calloc(1, sizeof(struct ssemd_args));
	parse_ssemd_args(argc, argv, args);

	size_t bytesToSend = getSize(args);

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

	struct admin_parser * adminParser = NULL;
	if(adminParser == NULL){
		adminParser = (struct admin_parser *)malloc(sizeof(struct admin_parser));
		admin_parser_init(adminParser);
	}

	//send command to server
	handleSend(args, sock, Buffer, bytesToSend, adminParser);

	memset(dataClient, 0, sizeof(uint8_t) * BUFFSIZE);

	//read reply
	handleRecv(sock, Buffer, adminParser);

	processParser(adminParser);

	close(sock);
	free(Buffer->data);
	free(Buffer);
	free(args);
	if(adminParser->data != NULL){
		free(adminParser->data);
	}
	free(adminParser);
	return 0;
}

void handleSend(struct ssemd_args *args, int sock, struct buffer * Buffer, size_t bytesToSend, struct admin_parser * adminParser){
	adminParser->type_sent = args->type;
	adminParser->cmd_sent = args->cmd;

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

	int bytesSent = send(sock, message, sizeof(message), 0);

	if(bytesSent < bytesToSend){
		print_log(ERROR, "ERROR, COULDN'T SEND ALL BYTES");
	}
}

void handleRecv(int sock, struct buffer * Buffer, struct admin_parser * adminParser){
	while (true) { //read REQUEST
		ssize_t bytesRecieved = recv(sock, Buffer->data, BUFFSIZE, 0);
		buffer_write_adv(Buffer, bytesRecieved);
		bool * errored = 0;
		if (bytesRecieved < 0) {
			print_log(ERROR, "recv() failed");
		} else {
			if(admin_consume(Buffer, adminParser, errored) == read_done){
				break;
			}
		}
	}
}

void processParser(struct admin_parser * adminParser){
	if(adminParser->status == SSEMD_ERROR && adminParser->response_code == SSEMD_ERROR_INCORRECT_TOKEN){
		print_log(ERROR, "Wrong token");
		return;
	}
	switch (adminParser->type_sent)
	{
	case SSEMD_GET:
		switch (adminParser->cmd_sent)
		{
		case SSEMD_HISTORIC_CONNECTIONS:
			if(adminParser->status == SSEMD_RESPONSE){
				print_log(INFO, "Quantity of historic connections: ");
				printInt(adminParser);
			} else if(adminParser->status == SSEMD_ERROR){
				print_log(INFO, "Couldn't get quantity of historic connections");
			}
			break;

		case SSEMD_CURRENT_CONNECTIONS:
			if(adminParser->status == SSEMD_RESPONSE){
				print_log(INFO, "Quantity of current connections: ");
				printInt(adminParser);
			} else if(adminParser->status == SSEMD_ERROR){
				print_log(INFO, "Couldn't get quantity of current connections");
			}
			break;
		
		case SSEMD_BYTES_TRANSFERRED:
			if(adminParser->status == SSEMD_RESPONSE){
				print_log(INFO, "Quantity of bytes transferred: ");
				printInt(adminParser);
			} else if(adminParser->status == SSEMD_ERROR){
				print_log(INFO, "Couldn't get quantity of bytes transferred");
			}
			break;

		case SSEMD_USER_LIST:
			if(adminParser->status == SSEMD_RESPONSE){
				print_log(INFO, "List of registered [user:password]:");
				printList(adminParser);
			} else if(adminParser->status == SSEMD_ERROR){
				print_log(INFO, "Couldn't get list of registered users:");
			}
			break;

		case SSEMD_DISSECTOR_STATUS:
			if(adminParser->status == SSEMD_RESPONSE){
				print_log(INFO, "Dissector is currently ");
				printBool(adminParser);
			} else if(adminParser->status == SSEMD_ERROR){
				print_log(INFO, "Couldn't get dissector status");
			}
			break;

		case SSEMD_AUTH_STATUS:
			if(adminParser->status == SSEMD_RESPONSE){
				print_log(INFO, "Authentication is currently ");
				printBool(adminParser);
			} else if(adminParser->status == SSEMD_ERROR){
				print_log(INFO, "Couldn't get authentication status");
			}
			break;

		case SSEMD_GET_BUFFER_SIZE:
			if(adminParser->status == SSEMD_RESPONSE){
				print_log(INFO, "Buffer size: ");
				printInt(adminParser);
			} else if(adminParser->status == SSEMD_ERROR){
				print_log(INFO, "Couldn't get buffer size");
			}
			break;

		case SSEMD_GET_TIMEOUT:
			if(adminParser->status == SSEMD_RESPONSE){
				print_log(INFO, "Timeout size: ");
				printInt(adminParser);
			} else if(adminParser->status == SSEMD_ERROR){
				print_log(INFO, "Couldn't get timeout");
			}
			break;
		
		default:
			break;
		}
		break;

	case SSEMD_EDIT:
		if(adminParser->status == SSEMD_RESPONSE){
			switch (adminParser->cmd_sent) 
			{
			case SSEMD_BUFFER_SIZE:
				print_log(INFO, "OK, setted new buffer size");
				break;

			case SSEMD_CLIENT_TIMEOUT:
				print_log(INFO, "OK, setted new timeout");
				break;
			
			case SSEMD_DISSECTOR_ON:
				print_log(INFO, "OK, dissector turned ON");
				break;

			case SSEMD_DISSECTOR_OFF:
				print_log(INFO, "OK, dissector turned OFF");
				break;

			case SSEMD_ADD_USER:
				print_log(INFO, "OK, added new user");
				break;

			case SSEMD_REMOVE_USER:
				print_log(INFO, "OK, user removed");
				break;

			case SSEMD_AUTH_ON:
				print_log(INFO, "OK, authentication turned ON");
				break;

			case SSEMD_AUTH_OFF:
				print_log(INFO, "OK, authentication turned OFF");
				break;
			}

		} else if(adminParser->status == SSEMD_ERROR){
			switch (adminParser->response_code)
			{
			case SSEMD_ERROR_SMALLBUFFER:
				print_log(ERROR, "Couldn't execute command, buffer size too small");
				break;

			case SSEMD_ERROR_BIGBUFFER:
				print_log(ERROR, "Couldn't execute command, buffer size too big");
				break;
			
			case SSEMD_ERROR_SMALLTIMEOUT:
				print_log(ERROR, "Couldn't execute command, timeout too small");
				break;

			case SSEMD_ERROR_BIGTIMEOUT:
				print_log(ERROR, "Couldn't execute command, timeout too big");
				break;
				
			case SSEMD_ERROR_NOSPACEUSER:
				print_log(ERROR, "Couldn't add new user, no space for new users");
				break;

			case SSEMD_ERROR_UNKNOWNTYPE:
				print_log(ERROR, "Couldn't execute command, unknown TYPE");
				break;

			case SSEMD_ERROR_UNKNOWNCMD:
				print_log(ERROR, "Couldn't execute command, tunknown CMD");
				break;

			case SSEMD_ERROR_NOSPACE:
				print_log(ERROR, "Couldn't execute command, no space to process command");
				break;

			case SSEMD_ERROR_UNKNOWNERROR:
			default:
				print_log(ERROR, "Couldn't execute command, unknown error");
				break;
			}
		}
	}
}


