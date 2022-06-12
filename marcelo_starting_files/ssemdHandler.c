#include "./include/ssemdHandler.h"

long valread;

// static const struct fd_handler ssemd = {
// 	.handle_read       = ssemd_read,
// 	.handle_write      = ssemd_write,
// 	.handle_close      = ssemd_close, // nada que liberar
// 	.handle_block	   = NULL,
// };

// void masterssemdHandler(struct selector_key *key) {
// 	const int new_client_socket = acceptTCPConnection(key->fd);
// 	selector_fd_set_nio(new_client_socket);
	
// 	// add new socket to array of sockets
// 	int i;
// 	struct clients_data * cli_data = (struct clients_data *)key->data;
// 	struct ssemd * clis = cli_data->clients;

// 	for (i = 0; i < cli_data->clients_size; i++) 
// 	{
// 		// if position is empty
// 		if(clis[i].isAvailable)
// 		{
// 			new_client(&clis[i], new_client_socket, BUFFSIZE);
			
// 			selector_register(key->s, new_client_socket, &ssemd, OP_READ, &clis[i]);

// 			print_log(DEBUG, "Adding client %d in socket %d\n" , i, new_client_socket);
// 			// print_log(DEBUG, "Adding remote socket to client %d in socket %d\n" , i, new_remote_socket);
// 			break;
// 		}
// 	}
// }

// void ssemd_read(struct selector_key *key) {
// 	struct ssemd * currClient = (struct ssemd *)key->data;
// 	struct connection_state currState = currClient->connection_state;
// 	switch (currState.client_state)
// 	{
// 		case HELLO_READ_STATE:
// 			hello_read(key);
// 			break;
// 		//Nunca entra aca porque estamos en lectura
// 		case HELLO_WRITE_STATE:
// 			break;
		
// 		case REQUEST_READ_STATE:
// 			request_read(key);
// 			break;

// 		case CONNECTED_STATE:
// 			read_connected_state(key);
// 			break;
// 		default:
// 			break;
// 	}
// }

// void ssemd_write(struct selector_key *key) {
// 	struct ssemd * currClient = (struct ssemd *)key->data;
// 	struct connection_state currState = currClient->connection_state;
// 	switch (currState.client_state) {
// 		//Nunca entra aca porque estamos en escritura
// 		case HELLO_READ_STATE:
// 			break;

// 		case HELLO_WRITE_STATE:
// 			hello_write(key);
// 		break;

// 		case REQUEST_CONNECTING_STATE:
// 			request_connecting(key);

// 		case REQUEST_WRITE_STATE:
// 			request_write(key);

// 		case REQUEST_READ_STATE:
// 			// do ssemd request read
// 			break;

// 		case CONNECTED_STATE:
// 			write_connected_state(key);
// 			break;
		
// 		default:
// 			break;
// 	}
// }

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
