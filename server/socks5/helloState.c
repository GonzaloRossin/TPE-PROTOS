#include "./../../include/helloState.h"

void hello_departure(struct socks5 * currClient) {
	// free(currClient->client.st_hello.pr->data);
	free(currClient->client.st_hello.pr);
}

void hello_read(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	hello_parser * pr = currClient->client.st_hello.pr;
	// Hello initialization
	if(!currClient->connection_state->init) {
		pr = (hello_parser *) calloc(1, sizeof(hello_parser));
		hello_parser_init(pr);
		// pr->on_authentication_method = on_auth;
		currClient->connection_state->init = true;
		currClient->client.st_hello.r = currClient->bufferFromClient;
		currClient->connection_state->on_departure = hello_departure;
		currClient->client.st_hello.w = currClient->bufferFromRemote;
		currClient->client.st_hello.pr = pr;
	}

	buffer * buff_r = currClient->client.st_hello.r;
	buffer * buff_w = currClient->client.st_hello.w;
	
	bool errored;

	size_t nbytes = buff_r->limit - buff_r->write;
	long valread = 0;
	if ((valread = read(key->fd , buff_r->data, nbytes)) <= 0) {
		print_log(INFO, "Host disconnected socket = %d \n", key->fd);
		selector_unregister_fd(key->s, key->fd);
		return ;
	} else {
		print_log(INFO, "Recieved %ld bytes from socket = %d\n", valread, key->fd);
		buffer_write_adv(buff_r, valread);
	}

	enum hello_state st = hello_consume(buff_r, pr, &errored);

	if (hello_is_done(st, &errored)) {
		hello_process(&currClient->client.st_hello);
		change_state(currClient, HELLO_WRITE_STATE);
		selector_set_interest(key->s, key->fd, OP_WRITE);
		struct users * users = get_users();
		fprintf(stdout, "%s\t%s\n", users[0].name, users[0].pass);
	} else if (st == hello_error) {

	}    
}

void hello_process(struct hello *d) {
	uint8_t *methods = d->pr->auth;
    uint8_t methods_c = d->pr->nauth;
	uint8_t m = SOCKS_HELLO_NO_ACCEPTABLE_METHODS;


    for (int i = 0; i < methods_c; i++)
    {    
        if(methods[i] == SOCKS_HELLO_USERPASS){
            m = SOCKS_HELLO_USERPASS;
            break;
        }
    }

	d->method = m;

	if (-1 == hello_marshall(d->w, m))
    {

    }
}

void hello_write(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;

	if(handleWrite(key->fd, currClient->client.st_hello.w) == 0){
		selector_set_interest(key->s, key->fd, OP_READ);
		currClient->connection_state->on_arrival = up_read_init;
		change_state(currClient, UP_READ_STATE);
	}
}

// void on_auth(struct hello_parser *parser, uint8_t method) {
// 	if (!parser->data) {
// 		u_int8_t * aux = (u_int8_t*) parser->data;
// 		aux = (uint8_t*)calloc(1, sizeof(uint8_t));
// 		*aux = method;
// 		parser->data = aux;
// 	}
// }

