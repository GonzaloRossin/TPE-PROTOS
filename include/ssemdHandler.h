#ifndef SSEMD_HANDLER_H_
#define SSEMD_HANDLER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>   
#include <arpa/inet.h>    
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> 
#include "logger.h"
#include "tcpServerUtil.h"
#include "buffer.h"
#include "helloState.h"
#include "client.h"
#include "selector.h"
#include "requestState.h"
#include "connectedState.h"
#include "ssemd.h"

void masterssemdHandler(struct selector_key *key);

void masterssemdHandler(struct selector_key *key);

void ssemd_read(struct selector_key *key);

void ssemd_write(struct selector_key *key);

void ssemd_close(struct selector_key *key);

void ssemd_change_state(struct ssemd * currClient, enum client_state state);

#endif


