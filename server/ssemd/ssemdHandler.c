#include "./../../include/ssemdHandler.h"

long valread;

static const struct fd_handler ssemdHandler = {
	.handle_read       = ssemd_read,
	.handle_write      = ssemd_write,
	.handle_close      = NULL, // ssemd_close, // nada que liberar
	.handle_block	   = NULL,
};

void masterssemdHandler(struct selector_key *key) {
	const int new_admin_socket = acceptTCPConnection(key->fd);
	selector_fd_set_nio(new_admin_socket);

    struct ssemd * admin = (struct ssemd *)key->data;

    if(admin->isAvailable) {
        new_admin(admin, new_admin_socket, BUFFSIZE);

        selector_register(key->s, new_admin_socket, &ssemdHandler, OP_READ, admin);
		print_log(DEBUG, "Adding admin in socket %d\n", new_admin_socket);

    } else {
        print_log(DEBUG, "error adminn\n");
    }
}

void ssemd_read(struct selector_key *key) {
	struct ssemd * currAdmin = (struct ssemd *)key->data;
	struct ssemd_connection_state currState = currAdmin->connection_state;
	switch (currState.ssemd_state)
	{
		case SSEMD_HELLO_READ_REQUEST:
			ssemd_read_request(key);
			break;
		//Nunca entra aca porque estamos en lectura
		case SSEMD_HELLO_WRITE_REQUEST:
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
		case SSEMD_HELLO_READ_REQUEST:
			break;
		case SSEMD_HELLO_WRITE_REQUEST:
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
	unsigned ret = SSEMD_HELLO_READ_REQUEST;
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
				currAdmin->connection_state.ssemd_state = SSEMD_HELLO_WRITE_REQUEST;
				selector_set_interest(key->s, key->fd, OP_WRITE);
			} else {
				ret = SSEMD_ERROR_STATE;
			}
		} else {
			// TODO: Enviar el error response_marshall();
		}
	} else {
		ret = SSEMD_ERROR_STATE;
	}
	// return error ? SSEMD_ERROR_STATE : ret;
}

void ssemd_request_process(struct ssemd * currAdmin) {
	payload * request = currAdmin->pr->data;

	if (0 == validate_token(currAdmin)) {
		if (request->type == SSEMD_GET) {
			ssemd_process_get(currAdmin);
		} else if (request->type == SSEMD_EDIT) {
			ssemd_process_edit(currAdmin);
		}
	} else {

	}
	
	
}

void ssemd_process_edit(struct ssemd * currAdmin) {
	ssemd_response * response = (ssemd_response *) malloc(sizeof(response));
	payload * request = currAdmin->pr->data;
	currAdmin->response = response;
	switch(request->CMD) {
		case SSEMD_BUFFER_SIZE: 

            break;
		case SSEMD_CLIENT_TIMEOUT: 
		
            break;
		case SSEMD_DISSECTOR_ON: 
		
            break;
		case SSEMD_DISSECTOR_OFF: 
		
            break;
		case SSEMD_ADD_USER: 
		
            break;
		case SSEMD_REMOVE_USER: 
		
            break;
		case SSEMD_AUTH_ON: 
		
            break;
		case SSEMD_AUTH_OFF: 
		
            break;
        default:
			break;
	}
}

void ssemd_process_get(struct ssemd * currAdmin) {
	ssemd_response * response = (ssemd_response *) malloc(sizeof(response));
	payload * request = currAdmin->pr->data;
	currAdmin->response = response;
	long c;
	switch(request->CMD) {
		case SSEMD_HISTORIC_CONNECTIONS: 
			c = get_historic_connections();
			memcpy(response->data, &c, sizeof(long));
            break;
		case SSEMD_CURRENT_CONNECTIONS: 
			c = get_current_connections();
			memcpy(response->data, &c, sizeof(long));
            break;
		case SSEMD_BYTES_TRANSFERRED: 
		
            break;
		case SSEMD_USER_LIST: 
		
            break;
		case SSEMD_DISSECTOR_STATUS: 
		
            break;
		case SSEMD_AUTH_STATUS: 
		
            break;
        default:
			break;
	}
	marshall(currAdmin->bufferWrite, currAdmin->response);
}

int validate_token(struct ssemd * currAdmin) {
	char *my_token = currAdmin->admin_token, *try_token = currAdmin->pr->data->token;
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

// void ssemd_close(struct selector_key *key) {
// 	struct ssemd * currClient = (struct ssemd *)key->data;

// 	if (currClient->client_socket == 0 || currClient->remote_socket == 0) {
// 		currClient->client.st_connected.init = 0;
// 		currClient->remote.st_connected.init = 0;

// 		free(currClient->bufferFromClient->data);
// 		free(currClient->bufferFromRemote->data);

// 		free(currClient->bufferFromClient);
// 		free(currClient->bufferFromRemote);

// 		memset(currClient, 0, sizeof(struct ssemd));
// 		currClient->connection_state.client_state = HELLO_READ_STATE;
// 		currClient->isAvailable = true;

// 	}
	
// 	if (key->fd == currClient->client_socket) {
// 		currClient->client_socket = 0;
// 	}
// 	if (key->fd == currClient->remote_socket) {
// 		currClient->remote_socket = 0;
// 	}
// 	close(key->fd);
// }

// void ssemd_change_state(struct ssemd * currClient, enum client_state state) {
// 	currClient->connection_state.client_state = state;
// 	currClient->connection_state.init = false;
// 	if (currClient->connection_state.on_departure != 0){
// 		currClient->connection_state.on_departure(currClient);
// 		currClient->connection_state.on_departure = NULL;
// 	}
// }
