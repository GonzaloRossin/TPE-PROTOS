#include "./../../include/ssemdHelloState.h"


void ssemd_hello_departure(struct ssemd * currAdmin) {
	free(currAdmin->admin.hello.pr->data);
	free(currAdmin->admin.hello.pr);
}

void ssemd_hello_read(struct selector_key *key) {
	struct ssemd * currAdmin = (struct ssemd *)key->data;
	hello_parser * pr = currAdmin->admin.hello.pr;
	// Hello initialization
	if(!currAdmin->connection_state.init) {
		pr = (hello_parser *) calloc(1, sizeof(hello_parser));
		hello_parser_init(pr);
		pr->on_authentication_method = on_auth;
		currAdmin->connection_state.init = true;
		currAdmin->admin.hello.r = currAdmin->bufferRead;
		currAdmin->connection_state.on_departure = hello_departure;
		currAdmin->admin.hello.w = currAdmin->bufferWrite;
		currAdmin->admin.hello.pr = pr;
	}

	buffer * buff_r = currAdmin->admin.hello.r;
	buffer * buff_w = currAdmin->admin.hello.w;
	
	bool errored;

	size_t nbytes = buff_r->limit - buff_r->write;
	long valread = 0;
	if ((valread = read( key->fd , buff_r->data, nbytes)) <= 0) {
		print_log(INFO, "Host disconnected socket = %d \n", key->fd);
		selector_unregister_fd(key->s, key->fd);
		return ;
	} else {
		buffer_write_adv(buff_r, valread);
	}

	enum hello_state st = hello_consume(buff_r, pr, &errored);

	if (st == hello_done) {
		u_int8_t * method = (u_int8_t*) pr->data;
		hello_marshall(buff_w, *method);
		change_state(currAdmin, HELLO_WRITE_STATE);
		selector_set_interest(key->s, key->fd, OP_WRITE);
	} else if (st == hello_error) {

	}    
}

void ssemd_hello_write(struct selector_key *key) {
	struct ssemd * currAdmin = (struct ssemd *)key->data;

	if(handleWrite(key->fd, currAdmin->admin.hello.w) == 0){
		selector_set_interest(key->s, key->fd, OP_READ);
		change_state(currAdmin, REQUEST_READ_STATE);
	}
}

void ssemd_on_auth(struct hello_parser *parser, uint8_t method) {
	if (!parser->data) {
		u_int8_t * aux = (u_int8_t*) parser->data;
		aux = (uint8_t*)calloc(1, sizeof(uint8_t));
		*aux = method;
		parser->data = aux;
	}
}

