#ifndef ADMIN_FUNCTIONS_H_
#define ADMIN_FUNCTIONS_H_

#include "util.h"

//GET:

char *
historic_connections();

int
current_connections();

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
