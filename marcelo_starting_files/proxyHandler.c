#include "./include/proxyHandler.h"


long valread;

static const struct fd_handler socksv5 = {
	.handle_read       = socks5_read,
	.handle_write      = socks5_write,
	.handle_close      = socks5_close, // nada que liberar
};

void masterSocketHandler(struct selector_key *key) {
	const int new_client_socket = acceptTCPConnection(key->fd);
	selector_fd_set_nio(new_client_socket);

	const int new_remote_socket = handleProxyAddr();
	selector_fd_set_nio(new_remote_socket);

	if ((new_remote_socket < 0)) {//open new remote socket
		print_log(ERROR, "Accept error on creating new remote socket %d", new_remote_socket);
	}

	// add new socket to array of sockets
	int i;
	struct clients_data * cli_data = (struct clients_data *)key->data;
	struct socks5 * clis = cli_data->clients;

	for (i = 0; i < cli_data->clients_size; i++) 
	{
		// if position is empty
		if(clis[i].isAvailable )
		{
			new_client(&clis[i], new_client_socket, BUFFSIZE);
			set_client_remote(&clis[i], new_remote_socket, BUFFSIZE);
			
			selector_register(key->s, new_client_socket, &socksv5, OP_READ, &clis[i]);
			selector_register(key->s, new_remote_socket, &socksv5, OP_READ, &clis[i]);

			print_log(DEBUG, "Adding client %d in socket %d\n" , i, new_client_socket);
			print_log(DEBUG, "Adding remote socket to client %d in socket %d\n" , i, new_remote_socket);

			break;
		}
	}
}

int handleProxyAddr() {
	char *server = "142.250.79.110"; //argv[1];     // First arg: server name IP address 

	// Third arg server port
	char * port = "80";//argv[3];

	// Create a reliable, stream socket using TCP
	int sock = tcpClientSocket(server, port);
	if (sock < 0) {
		print_log(FATAL, "socket() failed");
	}
	//print_log(DEBUG, "new (remote) socket is %d", sock);
	return sock;
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
	int clientSocket = currClient->client_socket;
	//int remoteSocket = currClient->remote_socket; //unused variable
	int fd_read, fd_write;
	buffer * buff;
	//esto es temporal:

	switch (currState.client_state)
	{
		case hello_read_state:
			hello_read(key);
			break;
		//Nunca entra aca porque estamos en lectura
		case hello_write_state:
			break;
		
		case request_read_state:
			request_read(key);
			break;

		case connected_state:
			connected_read(key);
			break;
		default:
			break;
	}
}

void socks5_write(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	struct connection_state currState = currClient->connection_state;
	int clientSocket = currClient->client_socket;
	//int remoteSocket = currClient->remote_socket; //unused variable
	int fd_read, fd_write;
	buffer * buff;

	switch (currState.client_state) {
		//Nunca entra aca porque estamos en escritura
		case hello_read_state:
			break;

		case hello_write_state:
			hello_write(key);
		break;

		case request_read_state:
			// do socks5 request read
			break;

		case connected_state:
			connected_write(key);
			break;
		
		default:
			break;
	}
}


void socks5_close(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;

	if (currClient->client_socket == 0 || currClient->remote_socket == 0) {
		currClient->client.st_connected.init = 0;
		currClient->remote.st_connected.init = 0;

		free(currClient->bufferFromClient->data);
		free(currClient->bufferFromRemote->data);

		free(currClient->bufferFromClient);
		free(currClient->bufferFromRemote);

		currClient->isAvailable = true;

		// close(currClient->client_socket);
		// close(currClient->remote_socket);
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
}



