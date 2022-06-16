#ifndef CONNECTED_STATE_H_
#define CONNECTED_STATE_H_


#include "socks5.h"
#include "selector.h"
#include "socks5Handler.h"
#include "pop3Parser.h"


void connected_init(struct socks5 * currClient);
void connected_close(struct selector_key *key);
void write_connected_state(struct selector_key *key);
void read_connected_state(struct selector_key *key);

#endif