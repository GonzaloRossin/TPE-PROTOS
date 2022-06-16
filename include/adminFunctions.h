#ifndef ADMIN_FUNCTIONS_H_
#define ADMIN_FUNCTIONS_H_

#include "util.h"
#include <string.h>
#include "logger.h"

//GET:
void register_client_connection();

unsigned int get_historic_connections();

unsigned int get_current_connections();

void unregister_current_connection();

// char *
// list_users();

// bool
// dissector_status();

// bool
// authentication_status();

void register_bytes_transferred(ssize_t bytes);

unsigned int get_bytes_transferred();

unsigned int get_BUFFSIZE();

bool get_dissector_state();

void set_BUFFSIZE(unsigned int newSize);

char * get_ADMIN_TOKEN();

void set_ADMIN_TOKEN(char * newToken);


//EDIT:



#endif
