#include "../../include/adminFunctions.h"

static long historic_connections = 0;
static long current_connections = 0;


void register_client_connection() {
    current_connections++;
    historic_connections++;
}

long get_historic_connections() {
    return historic_connections;
}

long get_current_connections() {
    return current_connections;
}

void unregister_current_connection() {
    current_connections--;
}