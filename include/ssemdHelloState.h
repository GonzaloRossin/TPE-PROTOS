#ifndef SSEMD_HELLO_STATE_H_
#define SSEMD_HELLO_STATE_H_

#include "client.h"
#include "selector.h"
#include "ssemdHandler.h"

void ssemd_hello_read(struct selector_key *key);
void ssemd_hello_write(struct selector_key *key);
void ssemd_hello_departure(struct ssemd * currAdmin);
void ssemd_on_auth(struct hello_parser *parser, uint8_t method);


#endif