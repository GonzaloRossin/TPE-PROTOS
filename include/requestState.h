#ifndef REQUEST_STATE_H_
#define REQUEST_STATE_H_

#include "socks5.h"
#include "selector.h"
#include "socks5Handler.h"

void request_read_init(struct socks5 * currClient);
void request_read(struct selector_key *key);
enum client_state request_connect(struct selector_key *key);
void request_connecting(struct selector_key *key);
void set_next_ip(struct socks5 * currClient, struct addrinfo * addr);
//serializa en buff la/una respuesta al request
//Retorna la cantidad de bytes ocupados del buffer, o -1 si no había espacio suficiente.
int request_marshall(struct socks5 * currClient);
int request_error_marshall(struct socks5 * currClient);
void request_write(struct selector_key *key);
void request_resolve(struct selector_key *key);
void request_departure(struct socks5 * currClient);
uint8_t * get_port(struct socks5 * currClient);

#endif