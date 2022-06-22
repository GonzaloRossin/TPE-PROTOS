#include "./../../include/requestState.h"

static const struct fd_handler socksv5 = {
	.handle_read       = socks5_read,
	.handle_write      = socks5_write,
	.handle_close      = socks5_close,
	.handle_block	   = socks5_block,
};

unsigned int identify_protocol_type(uint8_t * port);

void printConnectionRegister(struct socks5* clientSocket){
	struct tm timeStamp = clientSocket->timeStamp;
	StringBuilder * stringBuilder = sb_create();
	char* strRegister = NULL;
	char aux[264];
	sprintf(aux,"%d-%02d-%02d %02d:%02d:%02d", timeStamp.tm_year + 1900, timeStamp.tm_mon + 1, timeStamp.tm_mday, timeStamp.tm_hour, timeStamp.tm_min, timeStamp.tm_sec);
	sb_append( stringBuilder,aux);
	sprintf(aux,"\t%s\t\tA\t\t%s\t\t",clientSocket->username,clientSocket->clientAddr);
	sb_append(stringBuilder, aux);
	switch (clientSocket->client.st_request.request->dest_addr_type)
	{
		case socks_req_addrtype_domain:{
			sprintf(aux,"%s:%d\t\t",clientSocket->client.st_request.request->dest_addr.fqdn,ntohs(clientSocket->client.st_request.request->dest_port));
			sb_append(stringBuilder, aux);
			break;
		}
		case socks_req_addrtype_ipv4:{
			char str[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(clientSocket->client.st_request.request->dest_addr.ipv4.sin_addr), str, INET_ADDRSTRLEN);
			sb_append(stringBuilder, str);
			sprintf(aux,":%d\t\t",ntohs(clientSocket->client.st_request.request->dest_port));
			sb_append(stringBuilder, aux);
			break;
		}
		case socks_req_addrtype_ipv6:{
			char str[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, &(clientSocket->client.st_request.request->dest_addr.ipv6.sin6_addr), str, INET6_ADDRSTRLEN);
			sb_append(stringBuilder, str);
			sprintf(aux,":\t%d\t\t\t",ntohs(clientSocket->client.st_request.request->dest_port));
			sb_append(stringBuilder, aux);
			break;
		}
		default:
			break;
	}
	sprintf(aux,"\t%d",clientSocket->client.st_request.state);
	sb_append(stringBuilder, aux);
	strRegister = sb_concat(stringBuilder);
	printf("%s\n",strRegister);
	sb_free(stringBuilder);
	free(strRegister);
}

void request_departure(struct socks5 * currClient) {
	free(currClient->client.st_request.pr->request);
	free(currClient->client.st_request.pr);
}

void request_read_init(struct socks5 * currClient) {
	st_request *d = &currClient->client.st_request;
	struct connection_state *c = currClient->connection_state;

	d->pr = (request_parser *) calloc(1, sizeof(request_parser));
	d->r = currClient->bufferFromClient;
	d->w = currClient->bufferFromRemote;

	request_parser_init(d->pr);
	c->init = true;
}

void request_read(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	st_request *d = &currClient->client.st_request;

	request_parser * pr = d->pr;
	buffer * buff_r = d->r;


	bool errored = false;
    long valread = 0;
	size_t nbytes = buff_r->limit - buff_r->write;
	if ((valread = read( key->fd , buff_r->data, nbytes)) <= 0) {
		print_log(INFO, "Host disconnected\n");
		socks5_done(key);
	} else {

		buffer_write_adv(buff_r, valread);
		enum request_state st = request_consume(buff_r, pr, &errored);

		if(request_is_done(st, &errored) && !errored) {
			d->request = pr->request;
			enum client_state state = process_request(key);
			change_state(currClient, state);
			
		}
		if (errored) {
			switch (st) {
				case request_error_unsupported_version:
				case request_error_unsupported_atyp:
				case request_error:
					currClient->client.st_request.state = status_general_SOCKLS_server_failure;
					break;
				default:
					break;
			}
			request_error_marshall(currClient);
			selector_set_interest(key->s, currClient->client_socket, OP_WRITE);
			change_state(currClient, REQUEST_WRITE_STATE);
		}
	}
}
enum client_state process_request(struct selector_key *key) { //procesamiento del request
	struct socks5 * currClient = (struct socks5 *)key->data;
	struct request* request = currClient->client.st_request.request;
	enum client_state ret;
	enum socks_response_status status = status_general_SOCKLS_server_failure;

	switch (request->cmd)
	{
	case socks_req_cmd_connect:
		switch (request->dest_addr_type) {

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
		}	
		default:
			status = status_address_type_not_supported;
			ret = REQUEST_WRITE_STATE;
			break;
		}
		break;
	case socks_req_cmd_bind:
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
			selector_status st = selector_register(key->s, request.origin_fd, &socksv5, OP_WRITE, key->data);
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
		currClient->client.st_request.state = status_general_SOCKLS_server_failure;
	} else {
		if (error == 0) {
			conn->origin_fd = key->fd;
			currClient->remote_socket = key->fd;
			currClient->client.st_request.state = status_succeeded;
			currClient->protocol_type = identify_protocol_type(get_port(currClient));
			currClient->origin_adrr_type = family_to_socks_addr_type(currClient->origin_addr.ss_family);
			memcpy(currClient->requestRegister,currClient->client.st_request.request,sizeof(struct request));
			freeaddrinfo(currClient->origin_resolution);
		} else {
			if (currClient->origin_resolution_current) {
				if (currClient->origin_resolution_current->ai_next) {
					currClient->remote_socket = key->fd;
					selector_unregister_fd(key->s, key->fd);
					currClient->origin_resolution_current = currClient->origin_resolution_current->ai_next;
					set_next_ip(currClient, currClient->origin_resolution_current);
					enum client_state state = request_connect(key);
					change_state(currClient, state);
					return ;
				} else {
					// Si no pudimos conectar y no hay mas ips para intentar mando el reply de error
					freeaddrinfo(currClient->origin_resolution);
					currClient->remote_socket = key->fd;
					currClient->client.st_request.state = errno_to_socks(error);
					request_error_marshall(currClient);
					selector_set_interest(key->s, currClient->client_socket, OP_WRITE);
					change_state(currClient, REQUEST_WRITE_STATE);
					return ;
				}			
			} else {
				currClient->remote_socket = key->fd;
				currClient->client.st_request.state = errno_to_socks(error);
				request_error_marshall(currClient);
				selector_set_interest(key->s, currClient->client_socket, OP_WRITE);
				change_state(currClient, REQUEST_WRITE_STATE);
				return ;
			}
		}
		if (-1 == request_marshall(currClient)) {
			
		} else {
			change_state(currClient, REQUEST_WRITE_STATE);
			selector_set_interest(key->s, currClient->client_socket, OP_WRITE);
		}
	}
}

uint8_t * get_port(struct socks5 * currClient) {
	struct sockaddr_storage *sin = (struct sockaddr_storage *)&currClient->origin_addr;

	if (sin->ss_family == AF_INET) {
		memcpy(currClient->origin_port, &((struct sockaddr_in*)sin)->sin_port, sizeof(((struct sockaddr_in*)sin))->sin_port);
	} else if (sin->ss_family == AF_INET6) {
		memcpy(currClient->origin_port, &((struct sockaddr_in6*)sin)->sin6_port, sizeof(((struct sockaddr_in6*)sin))->sin6_port);
	}
	return currClient->origin_port;
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
	return socks_req_addrtype_failed;
}

int request_error_marshall(struct socks5 * currClient) {
	size_t n;
    uint8_t * buff = buffer_write_ptr(currClient->client.st_request.w, &n);

    if(n<2){
        return -1;
    }
    buff[0] = 0x05;
    buff[1] = currClient->client.st_request.state;
	buff[2] = 0x00;  
    buff[3] = 0x00;
    buff[4] = 0x00;
    buff[5] = 0x00;
    buff[6] = 0x00;

	buffer_write_adv(currClient->client.st_request.w, 7);
	return 7;
}

int request_marshall(struct socks5 * currClient) {
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

	// Imprimo por pantalla el registro de la conexion
	printConnectionRegister(currClient);
	// Envio el reply
	if(handleWrite(currClient->client_socket, currClient->client.st_request.w) == 0) {

		// Si todo salio bien paso al estado connected

		if (currClient->client.st_request.state == status_succeeded) {
			selector_set_interest(key->s, key->fd, OP_READ);
			currClient->connection_state->on_departure = request_departure;
			currClient->connection_state->on_arrival = connected_init;
			change_state(currClient, CONNECTED_STATE);
		// Si hubo algun error finalizo conexion
		} else {
			socks5_done(key);
		}
	}
}

void request_resolve(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;

	if (currClient->origin_resolution == 0) {
		currClient->client.st_request.state = status_general_SOCKLS_server_failure;
	} else {
		currClient->origin_resolution_current = currClient->origin_resolution;
        set_next_ip(currClient, currClient->origin_resolution_current);
	}
	enum client_state state =  request_connect(key);
	change_state(currClient, state);
}

void set_next_ip(struct socks5 * currClient, struct addrinfo * addr) {
    currClient->origin_domain = addr->ai_family;
	currClient->origin_addr_len = addr->ai_addrlen;
	memcpy(&currClient->origin_addr, addr->ai_addr, addr->ai_addrlen);
}

unsigned int
identify_protocol_type(uint8_t * port){
    if(port == NULL){
        return PROT_UNIDENTIFIED;
    }
    else if(port[0] == 0 && port[1] == 110){
        return PROT_POP3;
    }
    return PROT_UNIDENTIFIED;
}