#include "./../../include/helloState.h"

void hello_departure(struct socks5 * currClient) {
	hello_parser_close(currClient->client.hello.pr);
	free(currClient->client.hello.pr);
}

void hello_read_init(struct socks5 * currClient) {
	hello_st *d = &currClient->client.hello;
	struct connection_state *c = currClient->connection_state;

	d->pr = (hello_parser *) calloc(1, sizeof(hello_parser));
	hello_parser_init(d->pr);
	d->r = currClient->bufferFromClient;
	d->w = currClient->bufferFromRemote;

	c->on_departure = hello_departure;
	c->init = true;
}

void hello_read(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	hello_st *d = &currClient->client.hello;
	buffer * buff_r = d->r;

	bool errored;
	size_t nbytes = buff_r->limit - buff_r->write;
	long valread = 0;
	if ((valread = read(key->fd , buff_r->data, nbytes)) <= 0) {
		socks5_done(key);
		return ;
	} else {
		buffer_write_adv(buff_r, valread);
	}

	enum hello_state st = hello_consume(buff_r, d->pr, &errored);

	if (hello_is_done(st, &errored)) {
		hello_process(&currClient->client.hello);
		change_state(currClient, HELLO_WRITE_STATE);
		selector_set_interest(key->s, key->fd, OP_WRITE);
	} else if (st == hello_error) {
		socks5_done(key);
	}    
}

void hello_process(struct hello_st *d) {
	uint8_t *methods = d->pr->auth;
    uint8_t methods_c = d->pr->nauth;
	uint8_t m = SOCKS_HELLO_NO_ACCEPTABLE_METHODS;

	// Conseguimos la configuracion actual
	bool auth_on = get_auth_state();

	
    for (int i = 0; i < methods_c; i++)
    {    
		if (auth_on) {
			if(methods[i] == SOCKS_HELLO_USERPASS) {
				m = SOCKS_HELLO_USERPASS;
				break;
        	}
		} else {
			if(methods[i] == SOCKS_HELLO_NOAUTHENTICATION_REQUIRED) {
				m = SOCKS_HELLO_NOAUTHENTICATION_REQUIRED;
				break;
        	}
		}
    }

	d->method = m;

	if (-1 == hello_marshall(d->w, m))
    {

    }
}

// Hello Write State
void hello_write(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;

	// Enviamos la respuesta al hello
	if(handleWrite(key->fd, currClient->client.hello.w) == 0){
		selector_set_interest(key->s, key->fd, OP_READ);

		// Pasamos a autenticar el usuario
		if (currClient->client.hello.method == SOCKS_HELLO_USERPASS) {
			currClient->connection_state->on_arrival = up_read_init;
			change_state(currClient, UP_READ_STATE);

		// Pasamos directo a leer el request
		} else if (currClient->client.hello.method == SOCKS_HELLO_NOAUTHENTICATION_REQUIRED) {
			currClient->connection_state->on_arrival = request_read_init;
			change_state(currClient, REQUEST_READ_STATE);
		
		// Luego de mandar el mensaje cerramos la conexion
		} else {
			socks5_done(key);
		}
	}
}

