#include "./include/helloState.h"

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
    long valread = 0;
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
			struct fd_handler * socksv5 = malloc(sizeof(struct fd_handler));
            socksv5->handle_read = socks5_read;
            socksv5->handle_write = socks5_write;
            socksv5->handle_close = socks5_close;
            socksv5->handle_block = socks5_block;

			st = selector_register(key->s, request.origin_fd, socksv5, OP_WRITE, key->data);
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