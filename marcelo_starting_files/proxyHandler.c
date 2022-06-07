#include "proxyHandler.h"


long valread;

struct proxyBuffer {
	char * buffer;
	size_t len;     // longitud del buffer
	size_t from;    // desde donde falta escribir
};


// static const struct fd_handler remote_socksv5 = {
// 	.handle_read       = socks5_active_read_remote,
// 	.handle_write      = socks5_active_write_remote,
// 	.handle_close      = NULL, // nada que liberar
// };

// static const struct fd_handler client_socksv5 = {
// 	.handle_read       = socks5_active_read_client,
// 	.handle_write      = socks5_active_write_client,
// 	.handle_close      = NULL, // nada que liberar
// };

static const struct fd_handler socksv5 = {
	.handle_read       = socks5_read,
	.handle_write      = socks5_write,
	.handle_close      = NULL, // nada que liberar
};

void masterSocketHandler(struct selector_key *key) {
	struct socks5 *cli;
	
	const int new_client_socket = acceptTCPConnection(key->fd);
	selector_fd_set_nio(new_client_socket);

	const int new_remote_socket = handleProxyAddr();
	selector_fd_set_nio(new_remote_socket);

	if ((new_remote_socket < 0)) {//open new remote socket
		log(ERROR, "Accept error on creating new remote socket %d", new_remote_socket);
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
			
			int ss = selector_register(key->s, new_client_socket, &socksv5, OP_READ, &clis[i]);
			selector_register(key->s, new_remote_socket, &socksv5, OP_READ, &clis[i]);

			log(DEBUG, "Adding client %d in socket %d\n" , i, new_client_socket);
			log(DEBUG, "Adding remote socket to client %d in socket %d\n" , i, new_remote_socket);

			break;
		}
	}
}

int handleProxyAddr() {
	char *server = "142.250.79.110"; //argv[1];     // First arg: server name IP address 
	char *echoString = "hola"; //argv[2]; // Second arg: string to echo

	// Third arg server port
	char * port = "80";//argv[3];

	// Create a reliable, stream socket using TCP
	int sock = tcpClientSocket(server, port);
	if (sock < 0) {
		log(FATAL, "socket() failed")
	}
	//log(DEBUG, "new (remote) socket is %d", sock);
	return sock;
}

// Hay algo para escribir?
// Si está listo para escribir, escribimos. El problema es que a pesar de tener buffer para poder
// escribir, tal vez no sea suficiente. Por ejemplo podría tener 100 bytes libres en el buffer de
// salida, pero le pido que mande 1000 bytes.Por lo que tenemos que hacer un send no bloqueante,
// verificando la cantidad de bytes que pudo consumir TCP.
int handleWrite(int socket, struct buffer * buffer) {
	size_t bytesToSend = buffer->write - buffer->read;
	log(DEBUG, "bytesToSend %zu", bytesToSend);
	if (bytesToSend > 0) {  // Puede estar listo para enviar, pero no tenemos nada para enviar
		log(INFO, "Trying to send %zu bytes to socket %d\n", bytesToSend, socket);
		size_t bytesSent = send(socket, buffer->data, bytesToSend,  MSG_DONTWAIT); 
		log(INFO, "Sent %zu bytes\n", bytesSent);

		if ( bytesSent < 0) {
			// Esto no deberia pasar ya que el socket estaba listo para escritura
			// TODO: manejar el error
			log(FATAL, "Error sending to socket %d", socket);
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
	int remoteSocket = currClient->remote_socket;
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
	
			if(key->fd == clientSocket) {
				if (!currClient->client.st_connected.init) {
					init_client_copy(currClient);
				}
				fd_read = currClient->client.st_connected.read_fd;
				fd_write = currClient->client.st_connected.write_fd;
				buff = currClient->client.st_connected.w;
			} else {
				if (!currClient->client.st_connected.init) {
					init_remote_copy(currClient);
				}
				fd_read = currClient->remote.st_connected.read_fd;
				fd_write = currClient->remote.st_connected.write_fd;
				buff = currClient->remote.st_connected.w;
			}
			log(DEBUG, "reading on socket %d", fd_read);
			//Check if it was for closing , and also read the incoming message
			size_t nbytes = buff->limit - buff->write;
			log(DEBUG, "available bytes to write in bufferFromClient: %zu", nbytes);
			if ((valread = read( fd_read , buff->data, nbytes)) <= 0) //hace write en el buffer
			{
				log(INFO, "Host disconnected\n");

				removeClient(currClient);
				selector_unregister_fd(key->s, currClient->client_socket);
			} 
			else {
				log(DEBUG, "Received %zu bytes from socket %d\n", valread, clientSocket);
				log(DEBUG, "%s", buff->data);
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
	int remoteSocket = currClient->remote_socket;
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
			if(key->fd == clientSocket) {
				if (!currClient->client.st_connected.init) {
					init_client_copy(currClient);
				}
				fd_read = currClient->client.st_connected.read_fd;
				fd_write = currClient->client.st_connected.write_fd;
				buff = currClient->client.st_connected.r;
			} else {
				if (!currClient->remote.st_connected.init) {
					init_remote_copy(currClient);
				}
				fd_read = currClient->remote.st_connected.read_fd;
				fd_write = currClient->remote.st_connected.write_fd;
				buff = currClient->remote.st_connected.r;
			}
			log(DEBUG, "trying to send client content to his remote socket");
			if(handleWrite(fd_write, buff) == 0){
				//ya se mandaron todos los bytes, solo queda leer
				selector_set_interest(key->s, fd_write, OP_READ);
			}
			break;
		
		default:
			break;
	}
}



/* void socks5_active_read_client(struct selector_key *key){
	struct socks5 * currClient = (struct socks5 *)key->data;
	enum client_state currState = currClient->state;
	int clientSocket = currClient->client_socket;
	int remoteSocket = currClient->remote_socket;
	//esto es temporal:
	currState = connected_state;

	switch (currState)
	{
		case socks_hello_state:
			// do socks5 hello read
			break;
		
		case socks_request_state:
			// do socks5 request read
			break;

		case connected_state:
			log(DEBUG, "reading client on socket %d", clientSocket);
			//Check if it was for closing , and also read the incoming message
			size_t nbytes = currClient->bufferFromClient.limit - currClient->bufferFromClient.write;
			log(DEBUG, "available bytes to write in bufferFromClient: %zu", nbytes);
			if ((valread = read( clientSocket , currClient->bufferFromClient.data, nbytes)) <= 0) //hace write en el buffer
			{
				log(INFO, "Host disconnected\n");

				removeClient(currClient);
				selector_unregister_fd(key->s, currClient->client_socket);
			} 
			else {
				log(DEBUG, "Received %zu bytes from socket %d\n", valread, clientSocket);
				log(DEBUG, "%s", currClient->bufferFromClient.data);
				// ya se almacena en el buffer con la funcion read de arriba
				buffer_write_adv(&currClient->bufferFromClient, valread);
				selector_set_interest(key->s, remoteSocket, OP_WRITE);
			}
			break;

		default:
			break;
	}
} */

// void socks5_active_write_remote(struct selector_key *key){
// 	struct socks5 * currClient = (struct socks5 *)key->data;
// 	enum client_state currState = currClient->state;
// 	int clientSocket = currClient->client_socket;
// 	int remoteSocket = currClient->remote_socket;
// 	//esto es temporal:
// 	currState = connected_state;

// 	switch (currState)
// 	{
// 		case socks_hello_state:
// 			// do socks5 hello read
// 			break;
		
// 		case socks_request_state:
// 			// do socks5 request read
// 			break;

// 		case connected_state:
// 			//write to remote
// 			log(DEBUG, "trying to send client content to his remote socket");
// 			if(handleWrite(remoteSocket, &currClient->bufferFromClient) == 0){
// 				//ya se mandaron todos los bytes, solo queda leer
// 				selector_set_interest(key->s, remoteSocket, OP_READ);
// 			}
// 			break;
		
// 		default:
// 			break;
// 	}
// }

//-----------

/* void socks5_active_read_remote(struct selector_key *key){
	struct socks5 * currClient = (struct socks5 *)key->data;
	enum client_state currState = currClient->state;
	int clientSocket = currClient->client_socket;
	int remoteSocket = currClient->remote_socket;
	//esto es temporal:
	currState = connected_state;

	switch (currState)
	{
		case socks_hello_state:
			// do socks5 hello read
			break;
		
		case socks_request_state:
			// do socks5 request read
			break;

		case connected_state:
			// do normal read
			//read from client
			//hacer función separada mas cheto
			log(DEBUG, "reading remote on socket %d", clientSocket);
			//Check if it was for closing , and also read the incoming message
			size_t nbytes = currClient->bufferFromClient.limit - currClient->bufferFromClient.write;
			log(DEBUG, "available bytes to write in bufferFromClient: %zu", nbytes);
			if ((valread = read( remoteSocket , currClient->bufferFromRemote.data, nbytes)) <= 0) //hace write en el buffer
			{
				//Somebody disconnected , get his details and print
				//getpeername(clientSocket , (struct sockaddr*)&address , (socklen_t*)&addrlen);
				//log(INFO, "Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
				log(INFO, "Host disconnected\n");

				removeClient(currClient);
			} 
			else {
				log(DEBUG, "Received %zu bytes from (remote) socket %d\n", valread, remoteSocket);
				log(DEBUG, "%s", currClient->bufferFromRemote.data);
				// ya se almacena en el buffer con la funcion read de arriba
				buffer_write_adv(&currClient->bufferFromRemote, valread);
				selector_set_interest(key->s, clientSocket, OP_WRITE);

			}
			break;

		default:
			break;
	}
}

void socks5_active_write_client(struct selector_key *key){
	struct socks5 * currClient = (struct socks5 *)key->data;
	enum client_state currState = currClient->state;
	int clientSocket = currClient->client_socket;
	int remoteSocket = currClient->remote_socket;
	//esto es temporal:
	currState = connected_state;

	switch (currState)
	{
		case socks_hello_state:
			// do socks5 hello read
			break;
		
		case socks_request_state:
			// do socks5 request read
			break;

		case connected_state:
			//write to remote
			log(DEBUG, "trying to send client content to his remote socket");
			if(handleWrite(clientSocket, &currClient->bufferFromRemote) == 0){
				//ya se mandaron todos los bytes, solo queda leer
				selector_set_interest(key->s, clientSocket, OP_READ);
			}
			break;
		
		default:
			break;
	}
} */