#ifndef HELLO_STATE_H_
#define HELLO_STATE_H_

#include "socks5.h"
#include "selector.h"
#include "socks5Handler.h"
#include "upState.h"

void hello_read(struct selector_key *key);
void hello_write(struct selector_key *key);
void hello_departure(struct socks5 * currClient);
void hello_process(struct hello *d);
void on_auth(struct hello_parser *parser, uint8_t method);

#endif