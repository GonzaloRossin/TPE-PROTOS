#include "./../../include/ssemdHandler.h"



static const struct fd_handler ssemdHandler = {
	.handle_read       = ssemd_read,
	.handle_write      = ssemd_write,
	.handle_close      = ssemd_close,
	.handle_block	   = NULL,
};

void masterssemdHandler(struct selector_key *key) {
	char * adminAddr = (char *)calloc(1, sizeof(char) * 128);
	const int new_admin_socket = acceptTCPConnection(key->fd, adminAddr);
	selector_fd_set_nio(new_admin_socket);

	int i;
	struct admins_data * admin_data = (struct admins_data *)key->data;
    struct ssemd * admin = admin_data->admins;

	for (i = 0; i < admin_data->admins_size; i++) {
		if(admin[i].isAvailable) {
			new_admin(&admin[i], new_admin_socket, get_BUFFSIZE(), adminAddr);

			selector_register(key->s, new_admin_socket, &ssemdHandler, OP_READ, &admin[i]);
			// print_log(DEBUG, "Adding admin in socket %d\n", new_admin_socket);
			break;
		}
	}
}

void ssemd_read(struct selector_key *key) {
	struct ssemd * currAdmin = (struct ssemd *)key->data;
	struct ssemd_connection_state currState = currAdmin->connection_state;
	switch (currState.ssemd_state)
	{
		case SSEMD_READ_REQUEST:
			ssemd_read_request(key);
			break;
		//Nunca entra aca porque estamos en lectura
		case SSEMD_WRITE_REQUEST:
			break;
		case SSEMD_ERROR_STATE:
			break;
		default:
			break;
	}
}

void ssemd_write(struct selector_key *key) {
	struct ssemd * currAdmin = (struct ssemd *)key->data;
	struct ssemd_connection_state currState = currAdmin->connection_state;

	switch (currState.ssemd_state) {
		//Nunca entra aca porque estamos en escritura
		case SSEMD_READ_REQUEST:
			break;
		case SSEMD_WRITE_REQUEST:
			ssemd_write_request(key);
			break;
		case SSEMD_ERROR_STATE:
			break;
		default:
			break;
	}
}

void ssemd_write_request(struct selector_key *key) {
	struct ssemd * currAdmin = (struct ssemd *)key->data;

	if(handleWrite(key->fd, currAdmin->bufferWrite) == 0){
		selector_unregister_fd(key->s, key->fd);
	}
}

void ssemd_read_request(struct selector_key *key) {
	struct ssemd * currAdmin = (struct ssemd *)key->data;

	read_request_init(currAdmin);

	buffer * r = currAdmin->bufferRead;
	size_t count;
	ssize_t n;
	bool error = false;
	uint8_t *ptr;
	ptr = buffer_write_ptr(r, &count);

	n = recv(key->fd, ptr, count, 0);
	if (n > 0) {
		buffer_write_adv(r, n);
		const enum protocol_state st = protocol_consume(currAdmin->bufferRead, currAdmin->pr, &error);
		if (protocol_is_done(st, &error) && !error) {
			if (SELECTOR_SUCCESS == selector_set_interest_key(key, OP_WRITE)) {
				ssemd_request_process(currAdmin);
				marshall(currAdmin->bufferWrite, currAdmin->response);
				currAdmin->connection_state.ssemd_state = SSEMD_WRITE_REQUEST;
				selector_set_interest(key->s, key->fd, OP_WRITE);
			} else {
			}
		} else {
			if(error){ //is always error here
				ssemd_process_error(currAdmin, st);
				marshall(currAdmin->bufferWrite, currAdmin->response);
				currAdmin->connection_state.ssemd_state = SSEMD_WRITE_REQUEST;
				selector_set_interest(key->s, key->fd, OP_WRITE);
			}
		}
	} else {
	}

}

void ssemd_process_error(struct ssemd * currAdmin, const enum protocol_state st){
	ssemd_response * response = (ssemd_response *) calloc(1, sizeof(ssemd_response));
	currAdmin->response = response;
	response->status = SSEMD_ERROR;
	response->size1 = 0x00;
	response->size2 = 0x00;
	switch (st)
	{
	case protocol_error_type:
		response->code = SSEMD_ERROR_UNKNOWNTYPE;
		break;

	case protocol_error_cmd:
		response->code = SSEMD_ERROR_UNKNOWNCMD;
		break;

	case protocol_error_size:
		response->code = SSEMD_ERROR_INCORRECTSIZE;
		break;
	
	default:
		break;
	}
}


void ssemd_request_process(struct ssemd * currAdmin) {
	payload * request = currAdmin->pr->data;

	if (0 == validate_token(currAdmin)) {
		if (request->type == SSEMD_GET) {
			ssemd_process_get(currAdmin);
		} else if (request->type == SSEMD_EDIT) {
			ssemd_process_edit(currAdmin);
		}
	} else { //incorrect token
		ssemd_incorrect_token(currAdmin);
	}
	protocol_parser_close(currAdmin->pr);
}

void ssemd_process_get(struct ssemd * currAdmin) {
	ssemd_response * response = (ssemd_response *) calloc(1, sizeof(ssemd_response));
	payload * request = currAdmin->pr->data;
	currAdmin->response = response;
	unsigned int c = 0;
	switch(request->CMD) {
		case SSEMD_HISTORIC_CONNECTIONS:
			setResponse(response, SSEMD_RESPONSE_INT);
			c = get_historic_connections();
			c = htonl(c);
			memcpy(response->data, &c, sizeof(unsigned int));
            break;

		case SSEMD_CURRENT_CONNECTIONS: 
			setResponse(response, SSEMD_RESPONSE_INT);
			c = get_current_connections();
			c = htonl(c);
			memcpy(response->data, &c, sizeof(unsigned int));
            break;

		case SSEMD_BYTES_TRANSFERRED: 
			setResponse(response, SSEMD_RESPONSE_INT);
			c = get_bytes_transferred();
			c = htonl(c);
			memcpy(response->data, &c, sizeof(unsigned int));
            break;

		case SSEMD_USER_LIST: 
			handleGetUserList(request, response);
            break;

		case SSEMD_DISSECTOR_STATUS: 
			setResponse(response, SSEMD_RESPONSE_BOOL);
			handleBoolResponse(request, response);
            break;

		case SSEMD_AUTH_STATUS: 
			setResponse(response, SSEMD_RESPONSE_BOOL);
			handleBoolResponse(request, response);
            break;

		case SSEMD_GET_BUFFER_SIZE: 
			setResponse(response, SSEMD_RESPONSE_INT);
			c = get_BUFFSIZE();
			c = htonl(c);
			memcpy(response->data, &c, sizeof(unsigned int));
            break;
		
		case SSEMD_GET_TIMEOUT: 
			setResponse(response, SSEMD_RESPONSE_INT);
			c = get_timeout()->tv_sec;
			c = htonl(c);
			memcpy(response->data, &c, sizeof(unsigned int));
            break;

        default:
			setResponse(response, 0x00);
			break;
	}
}


void ssemd_process_edit(struct ssemd * currAdmin) {
	ssemd_response * response = (ssemd_response *) calloc(1, sizeof(ssemd_response));
	payload * request = currAdmin->pr->data;
	currAdmin->response = response;
	response->size1 = 0x00;
	response->size2 = 0x00;
	switch(request->CMD) {
		case SSEMD_BUFFER_SIZE:
			handleSetInt(request, response);
            break;
		case SSEMD_CLIENT_TIMEOUT: 
			handleSetInt(request, response);
            break;
		case SSEMD_DISSECTOR_ON: 
			if(set_dissector_ON()){
				response->status = SSEMD_RESPONSE;
				response->code = SSEMD_RESPONSE_OK;
			} else {
				response->status = SSEMD_ERROR;
				response->code = 0xFF;
				free(response->data);
			}
            break;
		case SSEMD_DISSECTOR_OFF: 
			if(set_dissector_OFF()){
				response->status = SSEMD_RESPONSE;
				response->code = SSEMD_RESPONSE_OK;
			} else {
				response->status = SSEMD_ERROR;
				response->code = 0xFF;
				free(response->data);
			}
            break;
		case SSEMD_ADD_USER: 
			handleEditUser(request, response, false);
            break;
		case SSEMD_REMOVE_USER:
			handleEditUser(request, response, true);
			break;
		case SSEMD_AUTH_ON: 
			if(set_auth_ON()){
				response->status = SSEMD_RESPONSE;
				response->code = SSEMD_RESPONSE_OK;
			} else {
				response->status = SSEMD_ERROR;
				response->code = 0xFF;
				free(response->data);
			}
            break;
		case SSEMD_AUTH_OFF: 
			if(set_auth_OFF()){
				response->status = SSEMD_RESPONSE;
				response->code = SSEMD_RESPONSE_OK;
			} else {
				response->status = SSEMD_ERROR;
				response->code = 0xFF;
				free(response->data);
			}
            break;

        default:
			break;
	}
}

void ssemd_incorrect_token(struct ssemd * currAdmin) {
	ssemd_response * response = (ssemd_response *) calloc(1, sizeof(ssemd_response));
	// payload * request = currAdmin->pr->data;
	currAdmin->response = response;
	response->status = SSEMD_ERROR;
	response->code = SSEMD_ERROR_INCORRECT_TOKEN;
	response->size1 = 0x00;
	response->size2 = 0x00;
}

void handleEditUser(struct payload * request, ssemd_response * response, bool isRemove){
	struct users * users = get_users();
	int retCode = 2;
	char * name;
	char * pass;
	char * empty = '\0';
	int dataPointer = 0;
	int wordPointer = 0;
	int userNumber;
	// struct users * newUser = (struct user*)calloc(1, sizeof(struct users));
	struct users newUser;
	newUser.name = (char *)calloc(1, sizeof(uint8_t) * 21);
	newUser.pass = (char *)calloc(1, sizeof(uint8_t) * 21);

	for(userNumber = 0; userNumber < MAX_USERS; userNumber++){
		name = users[userNumber].name;
		pass = users[userNumber].pass;
		if(! (name != empty && pass != empty)){ //if is not a valid user, write it here
			while(request->data[dataPointer] != ':'){ //write username
				if(wordPointer > 19){
					retCode = 3;
					break;
				} else {
					newUser.name[wordPointer++] = request->data[dataPointer++];
				}
			}
			if(retCode == 3){
				break;
			}
			newUser.name[wordPointer++] = '\0';
			wordPointer = 0; dataPointer++;

			while(dataPointer < request->data_len){ //write password
				if(wordPointer > 19){
					retCode = 3;
					break;
				} else {
					newUser.pass[wordPointer++] = request->data[dataPointer++];
				}
			}
			if(retCode == 3){
				break;
			}
			newUser.pass[wordPointer++] = '\0';

			if(isRemove){
				if(findUser(newUser, isRemove)){
					retCode = 0;
				} else {
					retCode = 4;
				}
			} else {
				if(findUser(newUser, isRemove)){
					retCode = 1;
				} else {
					users[userNumber] = newUser;
					retCode = 0;
				}
			}
			break;
		}
	}
	if(retCode == 0){
		response->status = SSEMD_RESPONSE;
		response->code = SSEMD_RESPONSE_OK;
	} else if(retCode == 1){
		response->status = SSEMD_ERROR;
		response->code = SSEMD_ERROR_REPEATEDUSER;
	} else if(retCode == 2){
		response->status = SSEMD_ERROR;
		response->code = SSEMD_ERROR_NOSPACEUSER;
	} else if(retCode == 3){
		response->status = SSEMD_ERROR;
		response->code = SSEMD_ERROR_USERTOOBIG;
	} else if(retCode == 4){
		response->status = SSEMD_ERROR;
		response->code = SSEMD_ERROR_USERNOTFOUND;
	}
	response->size1 = 0x00;
	response->size2 = 0x00;
	return;
}

bool findUser(struct users findUser, bool andRemove){
    struct users *users = get_users();
    bool found = false;
	char * empty = '\0';

    int i = 0;
    while (i < MAX_USERS && !found) {
        if (users[i].name != empty) {
            if (0 == strcmp((const char *)users[i].name, findUser.name)) {
                if (users[i].pass != empty) {
                    if (0 == strcmp((const char *)users[i].pass, findUser.pass)) {
                        found = true;
						if(andRemove){
							users[i].name = empty;
							users[i].pass = empty;
						}
                    }
                }  
            }
        }
        i++;
    }

    return found;
}

void handleGetUserList(struct payload * request, ssemd_response * response){
	if(request->type == SSEMD_GET && request->CMD == SSEMD_USER_LIST){
		struct users * users = get_users();
		response->data = (uint8_t *)malloc(sizeof(uint8_t) * (3)); // minimum
		char * empty = '\0';
		char * name;
		char * pass;
		int dataPointer = 0;
		int wordPointer;
		int userNumber;
		for(userNumber = 0; userNumber < MAX_USERS; userNumber++){
			wordPointer = 0;
			name = users[userNumber].name;
			pass = users[userNumber].pass;
			if(name != empty && pass != empty){ //if is a valid user
				// size_t toMalloc = strlen(name) + strlen(pass) +2;
				response->data = realloc(response->data, sizeof(uint8_t) * (dataPointer + strlen(name) + strlen(pass) + 2)); // + : + \0
				while(name[wordPointer] != '\0'){
					response->data[dataPointer++] = name[wordPointer++]; 
				}
				response->data[dataPointer++] = 0x3A; //es el ":"
				wordPointer = 0;
				while(pass[wordPointer] != '\0'){
					response->data[dataPointer++] = pass[wordPointer++]; 
				}
				response->data[dataPointer++] = 0x00; //fin de este usuario
			}

		}

		if(dataPointer > 255){
			response->size1 = (uint8_t) (uint8_t)((dataPointer & 0xFF00) >> 8); //(dataPointer/255);//    (255 - dataPointer);
			response->size2 = (uint8_t) (uint8_t)(dataPointer & 0x00FF); //dataPointer;
		} else {
			response->size1 = 0x00;
			response->size2 = (uint8_t) dataPointer;
		}
		response->status = SSEMD_RESPONSE;
		response->code = SSEMD_RESPONSE_LIST;
		
	} else {
		response->status = SSEMD_ERROR;
		response->code = 0xFF;
	}
}


void handleBoolResponse(struct payload * request, ssemd_response * response){
	unsigned int c;
	if(request->CMD == SSEMD_DISSECTOR_STATUS){
		response->status = SSEMD_RESPONSE;
		response->code = SSEMD_RESPONSE_BOOL;
		if(get_dissector_state()){
			c = SSEMD_TRUE;
		} else {
			c = SSEMD_FALSE;
		}
		memcpy(response->data, &c, sizeof(unsigned char));
	} else if(request->CMD == SSEMD_AUTH_STATUS){
		response->status = SSEMD_RESPONSE;
		response->code = SSEMD_RESPONSE_BOOL;
		if(get_auth_state()){
			c = SSEMD_TRUE;
		} else {
			c = SSEMD_FALSE;
		}
		memcpy(response->data, &c, sizeof(unsigned char));
	} else {
		response->status = SSEMD_ERROR;
		response->code = SSEMD_ERROR_UNKNOWNTYPE;
		free(response->data);
	}
}

void handleSetInt(struct payload * request, ssemd_response * response){
	int i;
	double ret=0;
	double power;
	unsigned int number;
	for(i=0; i<request->data_len; i++){
		power = pow(10, request->data_len -i-1);
		number = request->data[i] - '0';
		ret +=  number * power;
	}

	if(ret <= 0){
		response->status = SSEMD_ERROR;
		response->code = SSEMD_ERROR_SMALLBUFFER;
		free(response->data);
	} else if(ret > 2048000){
		response->status = SSEMD_ERROR;
		response->code = SSEMD_ERROR_BIGBUFFER;
		free(response->data);
	} else {
		response->status = SSEMD_RESPONSE;
		response->code = SSEMD_RESPONSE_OK;
	}

	if(request->CMD == SSEMD_BUFFER_SIZE){
		if(ret <= 0){
			response->status = SSEMD_ERROR;
			response->code = SSEMD_ERROR_SMALLBUFFER;
			free(response->data);
		} else if(ret > 2048000){
			response->status = SSEMD_ERROR;
			response->code = SSEMD_ERROR_BIGBUFFER;
			free(response->data);
		} else {
			set_BUFFSIZE(ret);
			response->status = SSEMD_RESPONSE;
			response->code = SSEMD_RESPONSE_OK;
		}
	} else if(request->CMD == SSEMD_CLIENT_TIMEOUT){
		if(ret <= 0){
			response->status = SSEMD_ERROR;
			response->code = SSEMD_ERROR_SMALLTIMEOUT;
			free(response->data);
		} else if(ret > 2048000){
			response->status = SSEMD_ERROR;
			response->code = SSEMD_ERROR_BIGTIMEOUT;
			free(response->data);
		} else {
			set_timeout(ret);
			response->status = SSEMD_RESPONSE;
			response->code = SSEMD_RESPONSE_OK;
		}
	}
}

int validate_token(struct ssemd * currAdmin) {
	char *my_token = get_ADMIN_TOKEN();
	char *try_token = currAdmin->pr->data->token;
	if (0 == strcmp(my_token, try_token)) {
		return 0;
	} else {
		return SSEMD_ERROR_STATE;
	}
}

void read_request_init(struct ssemd * currAdmin) {
	if (currAdmin->connection_state.init == false) {
		currAdmin->pr = (protocol_parser *)calloc(1, sizeof(protocol_parser));
		protocol_parser_init(currAdmin->pr);
		currAdmin->connection_state.init = true;
	}
}

int marshall(buffer * buffer, ssemd_response * response){
	unsigned int size = 0; //bytes for data
	size+=response->size2; 
	if(response->size1 != 0x00){
		size+=response->size1 + 255;
	} 

	size_t max_write;
    uint8_t * buff = buffer_write_ptr(buffer, &max_write);
	if(max_write < 4+size){
		response->status = SSEMD_ERROR;
        response->code = SSEMD_ERROR_NOSPACE;
		free(response->data);
    }
	int i=0;
	buff[i++] = response->status; //STATUS
	buff[i++] = response->code; //CODE
	buff[i++] = response->size1; //size1
	buff[i++] = response->size2; //size2

	if(size > 0){ //DATA
		unsigned int n = 0;
		while(n<size){
			buff[i++] = response->data[n++];
		}
		free(response->data);
	}
	buffer_write_adv(buffer, 4+size);
	free(response);
	return 4+size;
}

void setResponse(ssemd_response * response, uint8_t code){
	response->status = SSEMD_RESPONSE;
	switch (code)
	{
	case SSEMD_RESPONSE_OK:
		response->code=code;
		response->size1=0x00;
		response->size2=0x00;
		break;
	case SSEMD_RESPONSE_LIST:
		response->code=code;
		break;
	case SSEMD_RESPONSE_INT:
		response->data = (uint8_t*) calloc(1, sizeof(unsigned int));
		response->code=code;
		response->size1=0x00;
		response->size2=0x04;
		break;
	case SSEMD_RESPONSE_BOOL:
		response->data = (uint8_t*) calloc(1, sizeof(uint8_t));
		response->code=code;
		response->size1=0x00;
		response->size2=0x01;
		break;
	
	default:
		response->status = SSEMD_ERROR;
		response->code = SSEMD_ERROR_UNKNOWNTYPE;
		response->size1=0x00;
		response->size2=0x00;
		free(response->data);
		break;
	}
}

void ssemd_close(struct selector_key *key) {
	struct ssemd * currAdmin = (struct ssemd *)key->data;

	free(currAdmin->adminAddr);

	free(currAdmin->bufferRead->data);
	free(currAdmin->bufferWrite->data);

	free(currAdmin->bufferRead);
	free(currAdmin->bufferWrite);

	memset(currAdmin, 0, sizeof(struct ssemd));
	currAdmin->connection_state.ssemd_state = SSEMD_READ_REQUEST;
	currAdmin->isAvailable = true;

	close(key->fd);
}