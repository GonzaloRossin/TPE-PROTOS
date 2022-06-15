#include "./../../include/socks5Handler.h"


long valread;

static const struct fd_handler socksv5 = {
	.handle_read       = socks5_read,
	.handle_write      = socks5_write,
	.handle_close      = socks5_close, // nada que liberar
	.handle_block	   = socks5_block,
};

void masterSocks5Handler(struct selector_key *key) {
	const int new_client_socket = acceptTCPConnection(key->fd);
	selector_fd_set_nio(new_client_socket);
	
	// add new socket to array of sockets
	int i;
	struct clients_data * cli_data = (struct clients_data *)key->data;
	struct socks5 * clis = cli_data->clients;

	for (i = 0; i < cli_data->clients_size; i++) 
	{
		// if position is empty
		if(clis[i].isAvailable)
		{
			new_client(&clis[i], new_client_socket, BUFFSIZE);

			register_client_connection();
			
			selector_register(key->s, new_client_socket, &socksv5, OP_READ, &clis[i]);

			print_log(DEBUG, "Adding client %d in socket %d\n" , i, new_client_socket);
			// print_log(DEBUG, "Adding remote socket to client %d in socket %d\n" , i, new_remote_socket);

			time_t t = time(NULL);
  			struct tm tm = *localtime(&t);
 			print_log(INFO, "now: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
			print_log(INFO, "fecha: %s\tnombre\tip:puerto origen\tip:puerto destino\tstatus code\n", asctime(timeinfo));
			break;
		}
	}
}

// Escribo buffer en el socket
int handleWrite(int socket, struct buffer * buffer) {
	size_t bytesToSend = buffer->write - buffer->read;
	print_log(DEBUG, "bytesToSend %zu", bytesToSend);
	if (bytesToSend > 0) {  // Puede estar listo para enviar, pero no tenemos nada para enviar
		print_log(INFO, "Trying to send %zu bytes to socket %d\n", bytesToSend, socket);
		ssize_t bytesSent = send(socket, buffer->data, bytesToSend,  MSG_DONTWAIT); 
		print_log(INFO, "Sent %zu bytes\n", bytesSent);

		if ( bytesSent < 0) {
			// Esto no deberia pasar ya que el socket estaba listo para escritura
			// TODO: manejar el error
			print_log(FATAL, "Error sending to socket %d", socket);
			return -1;
		} else {
			size_t bytesLeft = bytesSent - bytesToSend;
			buffer_read_adv(buffer, bytesSent);

			return bytesLeft;
		}
	}
	return 0;
}


void socks5_read(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	struct connection_state currState = currClient->connection_state;
	switch (currState.client_state)
	{
		case HELLO_READ_STATE:
			hello_read(key);
			break;
		//Nunca entra aca porque estamos en lectura
		case HELLO_WRITE_STATE:
			break;
		
		case REQUEST_READ_STATE:
			request_read(key);
			break;

		case CONNECTED_STATE:
			read_connected_state(key);
			break;
		default:
			break;
	}
}

void socks5_write(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	struct connection_state currState = currClient->connection_state;
	switch (currState.client_state) {
		//Nunca entra aca porque estamos en escritura
		case HELLO_READ_STATE:
			break;

		case HELLO_WRITE_STATE:
			hello_write(key);
		break;

		case REQUEST_CONNECTING_STATE:
			request_connecting(key);
		break;
		case REQUEST_WRITE_STATE:
			request_write(key);
		break;
		case REQUEST_READ_STATE:
			// do socks5 request read
			break;

		case CONNECTED_STATE:
			write_connected_state(key);
			break;
		
		default:
			break;
	}
}

void socks5_close(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;

	if (currClient->client_socket == 0 || currClient->remote_socket == 0) {
		// currClient->client.st_connected.init = 0;
		// currClient->remote.st_connected.init = 0;

		free(currClient->bufferFromClient->data);
		free(currClient->bufferFromRemote->data);

		free(currClient->bufferFromClient);
		free(currClient->bufferFromRemote);

		memset(currClient, 0, sizeof(struct socks5));
		currClient->connection_state.client_state = HELLO_READ_STATE;
		currClient->isAvailable = true;

		void unregister_current_connection();

	}
	
	if (key->fd == currClient->client_socket) {
		currClient->client_socket = 0;
	}
	if (key->fd == currClient->remote_socket) {
		currClient->remote_socket = 0;
	}
	close(key->fd);
}

void change_state(struct socks5 * currClient, enum client_state state) {
	currClient->connection_state.client_state = state;
	currClient->connection_state.init = false;
	if (currClient->connection_state.on_departure != 0) {
		currClient->connection_state.on_departure(currClient);
		currClient->connection_state.on_departure = NULL;
	}
	if (currClient->connection_state.on_arrival != 0) {
		currClient->connection_state.on_arrival(currClient);
		currClient->connection_state.on_arrival = NULL;
	}
}

void socks5_block(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	struct connection_state currState = currClient->connection_state;

	switch (currState.client_state)
	{
		case HELLO_READ_STATE:
			break;
		//Nunca entra aca porque estamos en lectura
		case HELLO_WRITE_STATE:
			break;
		case REQUEST_RESOLVE_STATE:
			request_resolve(key);
		case REQUEST_READ_STATE:
			break;

		case CONNECTED_STATE:
			break;
		default:
			break;
	}
}

void socks5_done(struct selector_key *key) {
	const int fds[] = {
        ((struct socks5 *)key->data)->client_socket,
        ((struct socks5 *)key->data)->remote_socket
    };
    for (unsigned i = 0; i < N(fds); i++)
    {
        if (fds[i] != -1)
        {
            if (SELECTOR_SUCCESS != selector_unregister_fd(key->s, fds[i]))
            {
                printf("Socks is done for %d\n", fds[i]);
                abort();
            }
        }
    }
}

void * request_resolv_blocking(void *data) {
	struct selector_key *key = (struct selector_key *) data;
	struct socks5 * currClient = (struct socks5 *)key->data;

	pthread_detach(pthread_self());
	currClient->origin_resolution = 0;
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
		.ai_flags = AI_PASSIVE,
		.ai_protocol = 0,
		.ai_canonname = NULL,
		.ai_addr = NULL,
		.ai_next = NULL,
	};

	char buff[7];

	snprintf(buff, sizeof(buff), "%d", ntohs(currClient->client.st_request.pr->request->dest_port));

	getaddrinfo(currClient->client.st_request.pr->request->dest_addr.fqdn, buff, &hints, &currClient->origin_resolution);

	selector_notify_block(key->s, key->fd);

	free(data);

	return 0;
}


