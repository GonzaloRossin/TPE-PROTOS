#include "./include/connectedState.h"

void write_connected_state(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	int clientSocket = currClient->client_socket;
	int remoteSocket = currClient->remote_socket;
	int fd_read, fd_write;
	buffer * buff;
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
	} else if (key->fd == remoteSocket) {
		fd_read = currClient->remote.st_connected.write_fd;
		fd_write = currClient->client.st_connected.write_fd;
		buff = currClient->remote.st_connected.r;
	} else {
		// Morir tambien
	}
	print_log(DEBUG, "trying to send content recieved from socket %d to socket %d", fd_read, fd_write);
	if(handleWrite(fd_write, buff) == 0){
		//ya se mandaron todos los bytes, solo queda leer
		selector_set_interest(key->s, fd_write, OP_READ);
	}
}

void read_connected_state(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	int clientSocket = currClient->client_socket;
	int remoteSocket = currClient->remote_socket;
	int fd_read, fd_write;
	buffer * buff;

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
	} else if (key->fd == remoteSocket) {
		fd_read = currClient->remote.st_connected.read_fd;
		fd_write = currClient->client.st_connected.read_fd;
		buff = currClient->remote.st_connected.w;
	} else {
		// Morir
	}

	print_log(DEBUG, "reading on socket %d", fd_read);
	size_t nbytes = buff->limit - buff->write;
	print_log(DEBUG, "available bytes to write in bufferFromClient: %zu", nbytes);
	long valread = 0;
	if ((valread = read( fd_read , buff->data, nbytes)) <= 0) //hace write en el buffer
	{
		print_log(INFO, "Host disconnected\n");
		selector_unregister_fd(key->s, fd_read);
		selector_unregister_fd(key->s, fd_write);
	} 
	else {
		//Meter acá el parser POP3 
		print_log(DEBUG, "Recieved %zu bytes from socket %d\n", valread, fd_read);
		// ya se almacena en el buffer con la funcion read de arriba
		buffer_write_adv(buff, valread);
		selector_set_interest(key->s, fd_write, OP_WRITE);
	}
}