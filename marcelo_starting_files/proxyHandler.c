#include "./include/proxyHandler.h"


long valread;

static const struct fd_handler socksv5 = {
	.handle_read       = socks5_read,
	.handle_write      = socks5_write,
	.handle_close      = socks5_close, // nada que liberar
	.handle_block	   = socks5_block,
};

void masterSocketHandler(struct selector_key *key) {
	const int new_client_socket = acceptTCPConnection(key->fd);
	selector_fd_set_nio(new_client_socket);

	// const int new_remote_socket = handleProxyAddr();
	// selector_fd_set_nio(new_remote_socket);

	// if ((new_remote_socket < 0)) {//open new remote socket
	// 	print_log(ERROR, "Accept error on creating new remote socket %d", new_remote_socket);
	// }

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
			// set_client_remote(&clis[i], new_remote_socket, BUFFSIZE);
			
			selector_register(key->s, new_client_socket, &socksv5, OP_READ, &clis[i]);
			// selector_register(key->s, new_remote_socket, &socksv5, OP_READ, &clis[i]);

			print_log(DEBUG, "Adding client %d in socket %d\n" , i, new_client_socket);
			// print_log(DEBUG, "Adding remote socket to client %d in socket %d\n" , i, new_remote_socket);

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

		case REQUEST_WRITE_STATE:
			request_write(key);

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
		currClient->client.st_connected.init = 0;
		currClient->remote.st_connected.init = 0;

		free(currClient->bufferFromClient->data);
		free(currClient->bufferFromRemote->data);

		free(currClient->bufferFromClient);
		free(currClient->bufferFromRemote);

		memset(currClient, 0, sizeof(struct socks5));
		currClient->connection_state.client_state = HELLO_READ_STATE;
		currClient->isAvailable = true;

	}
	
	if (key->fd == currClient->client_socket) {
		currClient->client_socket = 0;
	}
	if (key->fd == currClient->remote_socket) {
		currClient->remote_socket = 0;
	}
	close(key->fd);
}

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
	if ((valread = read( fd_read , buff->data, nbytes)) <= 0) //hace write en el buffer
	{
		print_log(INFO, "Host disconnected\n");
		selector_unregister_fd(key->s, fd_read);
		selector_unregister_fd(key->s, fd_write);
	} 
	else {
		print_log(DEBUG, "Received %zu bytes from socket %d\n", valread, clientSocket);
		// ya se almacena en el buffer con la funcion read de arriba
		buffer_write_adv(buff, valread);
		selector_set_interest(key->s, fd_write, OP_WRITE);
	}
}

void change_state(struct socks5 * currClient, enum client_state state) {
	currClient->connection_state.client_state = state;
	currClient->connection_state.init = false;
	if (currClient->connection_state.on_departure != 0){
		currClient->connection_state.on_departure(currClient);
		currClient->connection_state.on_departure = NULL;
	}
}

void on_auth(struct hello_parser *parser, uint8_t method) {
	if (!parser->data) {
		u_int8_t * aux = (u_int8_t*) parser->data;
		aux = (uint8_t*)calloc(1, sizeof(uint8_t));
		*aux = method;
		parser->data = aux;
	}
}

void hello_departure(struct socks5 * currClient) {
	free(currClient->client.st_hello.pr->data);
	free(currClient->client.st_hello.pr);
}

void hello_read(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	hello_parser * pr = currClient->client.st_hello.pr;
	// Hello initialization
	if(!currClient->connection_state.init) {
		pr = (hello_parser *) calloc(1, sizeof(hello_parser));
		hello_parser_init(pr);
		pr->on_authentication_method = on_auth;
		currClient->connection_state.on_departure = hello_departure;
		currClient->connection_state.init = true;
		currClient->client.st_hello.r = currClient->bufferFromClient;
		currClient->client.st_hello.w = currClient->bufferFromRemote;
		currClient->client.st_hello.pr = pr;
	}

	buffer * buff_r = currClient->client.st_hello.r;
	buffer * buff_w = currClient->client.st_hello.w;
	
	bool errored;

	size_t nbytes = buff_r->limit - buff_r->write;
	if ((valread = read( key->fd , buff_r->data, nbytes)) <= 0) {
		print_log(INFO, "Host disconnected\n");
		selector_unregister_fd(key->s, key->fd);
	} else {
		buffer_write_adv(buff_r, valread);
	}

	enum hello_state st = hello_consume(buff_r, pr, &errored);

	if (st == hello_done) {
		u_int8_t * method = (u_int8_t*) pr->data;
		hello_marshall(buff_w, *method);
		change_state(currClient, HELLO_WRITE_STATE);
		selector_set_interest(key->s, key->fd, OP_WRITE);
	} else if (st == hello_error) {

	}    
}

void hello_write(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;

	if(handleWrite(key->fd, currClient->client.st_hello.w) == 0){
		selector_set_interest(key->s, key->fd, OP_READ);
		change_state(currClient, REQUEST_READ_STATE);
	}
}

void request_departure(struct socks5 * currClient) {
	free(currClient->client.st_request.pr->request);
	free(currClient->client.st_request.pr);
}

void request_read(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	request_parser * pr = currClient->client.st_request.pr;
	// Hello initialization
	if(!currClient->connection_state.init) {
		pr = (request_parser *) calloc(1, sizeof(request_parser)); //Limpiar mas tarde
		request_parser_init(pr);
		currClient->connection_state.init = true;
		currClient->connection_state.on_departure = request_departure;
		currClient->client.st_request.pr = pr;
		currClient->client.st_request.r = currClient->bufferFromClient;
		currClient->client.st_request.w = currClient->bufferFromRemote;
	}

	buffer * buff_r = currClient->client.st_request.r;
	bool errored;

	size_t nbytes = buff_r->limit - buff_r->write;
	if ((valread = read( key->fd , buff_r->data, nbytes)) <= 0) {
		print_log(INFO, "Host disconnected\n");
		selector_unregister_fd(key->s, key->fd);
	} else {
		buffer_write_adv(buff_r, valread);
		enum request_state st = request_consume(buff_r, pr, &errored);
		if(st == request_done) {
			currClient->client.st_request.request = pr->request;
			enum client_state state = process_request(key);
			change_state(currClient, state);
		}
	}
}
enum client_state process_request(struct selector_key *key){ //procesamiento del request
	struct socks5 * currClient = (struct socks5 *)key->data;
	struct request* request = currClient->client.st_request.request;
	enum client_state ret;
	enum socks_response_status status = status_general_SOCKLS_server_failure;

	switch (request->cmd)
	{
	case socks_req_cmd_connect:
		switch (request->dest_addr_type)
		{
		case socks_req_addrtype_ipv4:{
			currClient->origin_domain = AF_INET;
			request->dest_addr.ipv4.sin_port = request->dest_port;
			currClient->origin_addr_len = sizeof(request->dest_addr.ipv4);
			memcpy(&currClient->origin_addr, &request->dest_addr, sizeof(request->dest_addr.ipv4));
			ret = request_connect(key);
			break;
		}
	
		case socks_req_addrtype_ipv6:
			currClient->origin_domain = AF_INET6;
			request->dest_addr.ipv6.sin6_port = request->dest_port;
			currClient->origin_addr_len = sizeof(request->dest_addr.ipv6);
			memcpy(&currClient->origin_addr, &request->dest_addr, sizeof(request->dest_addr.ipv6));
			ret = request_connect(key);
			break;

		case socks_req_addrtype_domain: {
			struct selector_key *k = (struct selector_key*) malloc(sizeof(*key));
			if (k == NULL) {
				ret = REQUEST_WRITE_STATE;
				status = status_general_SOCKLS_server_failure;
				selector_set_interest_key(key, OP_WRITE);
			} else {
				memcpy(k, key, sizeof(*key));
				pthread_t tid;
				if (-1 == pthread_create(&tid, 0, request_resolv_blocking, k)) {
					ret = REQUEST_WRITE_STATE;
					status = status_general_SOCKLS_server_failure;
					selector_set_interest_key(key, OP_WRITE);
				} else {
					ret = REQUEST_RESOLVE_STATE;
					selector_set_interest_key(key, OP_NOOP);
				}
			}
			break;
		
		}	default:
			status = status_address_type_not_supported;
			ret = REQUEST_WRITE_STATE;
			break;
		}

        // status = cmd_resolve(request, &originaddr, &origin_addr_len, &origin_domain);//a chequear
		break;
	case socks_req_cmd_bind:
		break;
	case socks_req_cmd_associate:
    default:
    	status = status_command_not_supported;
        break;
    }
	currClient->client.st_request.state = status;
	return ret;
}

enum client_state request_connect(struct selector_key *key) {
	bool error = false;

	struct socks5 * currClient = (struct socks5 *)key->data;
	struct st_request request = currClient->client.st_request;
	enum socks_response_status status = request.state;

	request.origin_fd = socket(currClient->origin_domain, SOCK_STREAM, 0);
	if (request.origin_fd == -1) {
		error = true;
		goto finally;
	}
	if (selector_fd_set_nio(request.origin_fd) == -1) {
		error = true;
		goto finally;
	}
	if (-1 == connect(request.origin_fd, (const struct sockaddr *)&currClient->origin_addr, currClient->origin_addr_len)) {
		if (errno == EINPROGRESS) {
			selector_status st = selector_set_interest_key(key, OP_NOOP);
			if(SELECTOR_SUCCESS != st) {
				error = true;
				goto finally;
			}
			
			st = selector_register(key->s, request.origin_fd, &socksv5, OP_WRITE, key->data);
			if (SELECTOR_SUCCESS != st) {
				close(request.origin_fd);
			}
		}
	} else {
		error = true;
		goto finally;
	}
finally:
	if (error) {
		if (request.origin_fd != -1) {
			close(request.origin_fd);
			request.origin_fd = -1;
		}
	}
	request.state = status;
	return REQUEST_CONNECTING_STATE;
}

void request_connecting(struct selector_key *key) {
	int error;
	socklen_t len = sizeof(error);
	struct socks5 * currClient = (struct socks5 *)key->data;
	struct connecting * conn = &currClient->remote.conn;

	if (getsockopt(key->fd, SOL_SOCKET, SO_ERROR, &error, &len)) {
		conn->client_state = status_general_SOCKLS_server_failure;
	} else {
		if (error == 0) {
			conn->client_state = status_succeeded;
			conn->origin_fd = key->fd;
			currClient->remote_socket = key->fd;
			currClient->client.st_request.state = status_succeeded;
			currClient->origin_adrr_type = family_to_socks_addr_type(currClient->origin_addr.ss_family);
		} else {
			
		}
	}

	if (-1 == request_marshall(currClient)) {
		
	} else {
		change_state(currClient, REQUEST_WRITE_STATE);
		selector_set_interest(key->s, currClient->client_socket, OP_WRITE);
	}
}

enum socks_addr_type family_to_socks_addr_type(int family) {
	switch (family)
	{
	case AF_INET:
		return socks_req_addrtype_ipv4;
		break;
	case AF_INET6:
		return socks_req_addrtype_ipv6;
		break;
	default:
		break;	
	}
	// Aca no llega no se que devolver
	return status_general_SOCKLS_server_failure;
}

int request_marshall(struct socks5 * currClient){
    size_t n;
    uint8_t * buff = buffer_write_ptr(currClient->client.st_request.w, &n);
	int addr_size = 0;

	struct sockaddr_storage *sin = (struct sockaddr_storage *)&currClient->origin_addr;
    if(n<2){
        return -1;
    }
    buff[0] = 0x05;
    buff[1] = currClient->client.st_request.state;
    buff[2] = 0x00;
	buff[3] = currClient->origin_adrr_type;
	switch (currClient->origin_adrr_type)
	{
	case socks_req_addrtype_ipv4:{
		addr_size += IP_V4_ADDR_SIZE;
		memcpy(buff + 4, (void *)&(((struct sockaddr_in*)sin)->sin_addr), IP_V4_ADDR_SIZE);
		memcpy(buff + 4 + IP_V4_ADDR_SIZE, (void *)&(((struct sockaddr_in*)sin)->sin_port), 2);
		break;
	}
		
	case socks_req_addrtype_ipv6:
		addr_size += IP_V6_ADDR_SIZE;
		memcpy(buff + 4, (void *)&(((struct sockaddr_in6*)sin)->sin6_addr), IP_V6_ADDR_SIZE);
		memcpy(buff + 4 + IP_V6_ADDR_SIZE, (void *)&(((struct sockaddr_in6*)sin)->sin6_port), 2);
		break;

	default:
		break;
	}
    buffer_write_adv(currClient->client.st_request.w, 4 + addr_size + 2);
    return 2;
}

void request_write(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;

	if(handleWrite(currClient->client_socket, currClient->client.st_request.w) == 0){
		selector_set_interest(key->s, key->fd, OP_READ);
		change_state(currClient, CONNECTED_STATE);
	}
}

void request_resolve(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;

	if (currClient->origin_resolution == 0) {
		currClient->client.st_request.state = status_general_SOCKLS_server_failure;
	} else {
		currClient->origin_domain = currClient->origin_resolution->ai_family;
		currClient->origin_addr_len = currClient->origin_resolution->ai_addrlen;
		memcpy(&currClient->origin_addr, currClient->origin_resolution->ai_addr, currClient->origin_resolution->ai_addrlen);
		freeaddrinfo(currClient->origin_resolution);
		currClient->origin_resolution = 0;
	}

	enum client_state state =  request_connect(key);
	change_state(currClient, state);
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


