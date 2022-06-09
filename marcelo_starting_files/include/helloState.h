#ifndef HELLO_STATE_H
#define HELLO_STATE_H

// #include "logger.h"
// #include "tcpServerUtil.h"
// #include "tcpClientUtil.h"
// #include "buffer.h"
// #include "client.h"
// #include "selector.h"
// #include "hello.h"
#include "proxyHandler.h"
void hello_read(struct selector_key *key);
void hello_write(struct selector_key *key);
void on_auth(struct hello_parser *parser, uint8_t method);

#endif