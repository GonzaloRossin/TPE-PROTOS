#include "proxyHandler.h"


long valread;

struct proxyBuffer {
	char * buffer;
	size_t len;     // longitud del buffer
	size_t from;    // desde donde falta escribir
};

static const struct fd_handler socksv5 = {
	.handle_read       = socks5_read,
	.handle_write      = socks5_write,
	.handle_close      = socks5_close, // nada que liberar
};

void masterSocketHandler(struct selector_key *key) {
	const int new_client_socket = acceptTCPConnection(key->fd);
	selector_fd_set_nio(new_client_socket);

	const int new_remote_socket = handleProxyAddr(key);
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

int handleProxyAddr(struct selector_key * key) {
	char *server = "142.250.79.110"; //argv[1];     // First arg: server name IP address 
	//char *echoString = "hola"; //argv[2]; // Second arg: string to echo

	// Third arg server port
	char * port = "80";//argv[3];

	// Create a reliable, stream socket using TCP
	int sock = tcpClientSocket(server, port, key, &socksv5);
	if (sock < 0) {
		print_log(FATAL, "socket() failed");
	}
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
	currState.client_state = connected_state;

	switch (currState.client_state)
	{
		case socks_hello_state:
			// do socks5 hello read
			break;
		
		case socks_request_state:
			// do socks5 request read
			break;

		case connected_state:
			if (!currClient->client.st_connected.init) {
				init_client_copy(currClient);
			}
			if (!currClient->remote.st_connected.init) {
				init_remote_copy(currClient);
			}
			if(key->fd == clientSocket) {
				fd_read = currClient->client.st_connected.read_fd;
				fd_write = currClient->remote.st_connected.read_fd;
				buff = currClient->client.st_connected.w;
			} else {
				fd_read = currClient->remote.st_connected.read_fd;
				fd_write = currClient->client.st_connected.read_fd;
				buff = currClient->remote.st_connected.w;
			}
			print_log(DEBUG, "reading on socket %d", fd_read);
			//Check if it was for closing , and also read the incoming message
			size_t nbytes = buff->limit - buff->write;
			print_log(DEBUG, "available bytes to write in bufferFromClient: %zu", nbytes);
			if ((valread = read( fd_read , buff->data, nbytes)) <= 0) //hace write en el buffer
			{
				print_log(INFO, "Host disconnected\n");
				// if (fd_write != 0) {
				// 	selector_set_interest(key->s, fd_write, OP_WRITE);	
				// }
				selector_unregister_fd(key->s, fd_read);
				selector_unregister_fd(key->s, fd_write);
			} 
			else {
				print_log(DEBUG, "Received %zu bytes from socket %d\n", valread, clientSocket);
				print_log(DEBUG, "%s", buff->data);
				// ya se almacena en el buffer con la funcion read de arriba
				buffer_write_adv(buff, valread);
				selector_set_interest(key->s, fd_write, OP_WRITE);
			}
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
	//esto es temporal:
	currState.client_state = connected_state;

	switch (currState.client_state) {
		case socks_hello_state:
			// do socks5 hello read
			break;
		
		case socks_request_state:
			// do socks5 request read
			break;

		case connected_state:
			if (!currClient->client.st_connected.init) {
				init_client_copy(currClient);
			}
			if (!currClient->remote.st_connected.init) {
				init_remote_copy(currClient);
			}
			if(key->fd == clientSocket) {
				fd_read = currClient->client.st_connected.write_fd;
				fd_write = currClient->remote.st_connected.write_fd;
				buff = currClient->client.st_connected.r;
			} else {
				fd_read = currClient->remote.st_connected.write_fd;
				fd_write = currClient->client.st_connected.write_fd;
				buff = currClient->remote.st_connected.r;
			}
			print_log(DEBUG, "trying to send content recieved from socket %d to socket %d", fd_read, fd_write);
			if(handleWrite(fd_write, buff) == 0){
				//ya se mandaron todos los bytes, solo queda leer
				selector_set_interest(key->s, fd_write, OP_READ);
			}
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
