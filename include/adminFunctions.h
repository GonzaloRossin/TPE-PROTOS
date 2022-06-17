#ifndef ADMIN_FUNCTIONS_H_
#define ADMIN_FUNCTIONS_H_

#include "util.h"
#include <string.h>
#include "logger.h"
#include "args.h"

//GET:
void
register_client_connection();

unsigned int
get_historic_connections();

unsigned int
get_current_connections();

void
unregister_current_connection();


void
register_bytes_transferred(unsigned int bytes);

unsigned int
get_bytes_transferred();

unsigned int
get_BUFFSIZE();

void
set_BUFFSIZE(unsigned int newSize);

char *
get_ADMIN_TOKEN();

void
set_ADMIN_TOKEN(char * newToken);

bool
get_dissector_state();

bool 
set_dissector_ON();

bool 
set_dissector_OFF();

bool 
get_auth_state();

bool 
set_auth_ON();

bool 
set_auth_OFF();

void 
init_users(struct users* new_users);

struct 
users * get_users();

//EDIT:



#endif
