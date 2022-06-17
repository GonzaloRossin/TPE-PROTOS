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
#include "socks5.h"
#include "selector.h"
#include "requestState.h"
#include "connectedState.h"
#include "protocolParser.h"
#include "ssemd.h"
#include "util.h"
#include <endian.h>
#include <math.h>

void masterssemdHandler(struct selector_key *key);

void ssemd_read(struct selector_key *key);

void ssemd_read_request(struct selector_key *key);
void read_request_init(struct ssemd * currAdmin);

void ssemd_request_process(struct ssemd * currAdmin);
int validate_token(struct ssemd * currAdmin);
void ssemd_process_get(struct ssemd * currAdmin);
void ssemd_process_edit(struct ssemd * currAdmin);

void ssemd_incorrect_token(struct ssemd * currAdmin);

void ssemd_write_request(struct selector_key *key);

void ssemd_write(struct selector_key *key);

void ssemd_close(struct selector_key *key);

void ssemd_change_state(struct ssemd * currAdmin, enum client_state state);

int marshall(buffer * buffer, ssemd_response * response);

void setResponse(ssemd_response * response, uint8_t code);

void handleSetInt(struct payload * request, ssemd_response * response);

void handleBoolResponse(struct payload * request, ssemd_response * response);

void handleGetUserList(struct payload * request, ssemd_response * response);

#endif


