#ifndef REQUEST_STATE_H
#define REQUEST_STATE_H

// #include "logger.h"
// #include "tcpServerUtil.h"
// #include "tcpClientUtil.h"
// #include "buffer.h"
// #include "client.h"
// #include "selector.h"
// #include "request.h"
#include "proxyHandler.h"

void request_read(struct selector_key *key);

#endif