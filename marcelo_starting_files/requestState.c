#include "./include/requestState.h"


void request_read(struct selector_key *key) {
	struct socks5 * currClient = (struct socks5 *)key->data;
	request_parser * pr = currClient->client.st_request.pr;
	// Hello initialization
	if(!currClient->connection_state.init) {
		pr = (request_parser *) calloc(1, sizeof(request_parser)); //Limpiar mas tarde
		request_parser_init(pr);
		currClient->connection_state.init = true;
		currClient->client.st_request.r = currClient->bufferFromClient;
	}

}