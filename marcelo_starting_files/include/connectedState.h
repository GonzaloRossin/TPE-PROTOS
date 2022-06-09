#ifndef CONNECTED_STATE_H
#define CONNECTED_STATE_H

// #include "logger.h"
// #include "tcpServerUtil.h"
// #include "tcpClientUtil.h"
// #include "buffer.h"
// #include "client.h"
// #include "selector.h"
#include "proxyHandler.h"

void connected_write(struct selector_key *key);
void connected_read(struct selector_key *key);

#endif