#include "./include/connectedState.h"

void connected_write(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	int clientSocket = currClient->client_socket;
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
}

void connected_read(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	int clientSocket = currClient->client_socket;
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
	} else {
		fd_read = currClient->remote.st_connected.read_fd;
		fd_write = currClient->client.st_connected.read_fd;
		buff = currClient->remote.st_connected.w;
	}
	print_log(DEBUG, "reading on socket %d", fd_read);
	//Check if it was for closing , and also read the incoming message
	size_t nbytes = buff->limit - buff->write;
	print_log(DEBUG, "available bytes to write in bufferFromClient: %zu", nbytes);
	long valread;
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
}