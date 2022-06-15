#include "../../include/adminFunctions.h"

static unsigned long historic_connections = 0;
static unsigned long current_connections = 0;


void register_client_connection() {
    current_connections++;
    historic_connections++;
}

unsigned long get_historic_connections() {
    return historic_connections;
}

unsigned long get_current_connections() {
    return current_connections;
}

void unregister_current_connection() {
    current_connections--;
}