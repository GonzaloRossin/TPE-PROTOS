#ifndef ADMIN_FUNCTIONS_H_
#define ADMIN_FUNCTIONS_H_

#include "util.h"

//GET:
void register_client_connection();

unsigned long get_historic_connections();

unsigned long get_current_connections();

void unregister_current_connection();

long
bytes_transferred();

char *
list_users();

bool
dissector_status();

bool
authentication_status();


//EDIT:



#endif
