#ifndef REQUEST_STATE_H_
#define REQUEST_STATE_H_

#include "client.h"
#include "selector.h"
#include "proxyHandler.h"
    
void request_read(struct selector_key *key);
enum client_state request_connect(struct selector_key *key);
void request_connecting(struct selector_key *key);
//serializa en buff la/una respuesta al request
//Retorna la cantidad de bytes ocupados del buffer, o -1 si no había espacio suficiente.
int request_marshall(struct socks5 * currClient);
void request_write(struct selector_key *key);
void request_resolve(struct selector_key *key);
void request_departure(struct socks5 * currClient);

#endif