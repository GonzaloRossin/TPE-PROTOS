#ifndef CONNECTED_STATE_H_
#define CONNECTED_STATE_H_


#include "client.h"
#include "selector.h"
#include "socks5Handler.h"

void write_connected_state(struct selector_key *key);
void read_connected_state(struct selector_key *key);

#endif