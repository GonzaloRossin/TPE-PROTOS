#include "../../include/adminFunctions.h"

static unsigned int historic_connections = 0;
static unsigned int current_connections = 0;
static unsigned int bytes_transferred = 0;

static unsigned int BUFFSIZE = 4096;

static char ADMIN_TOKEN[100];


void register_client_connection() {
    current_connections++;
    historic_connections++;
}

unsigned int get_historic_connections() {
    return historic_connections;
}

unsigned int get_current_connections() {
    return current_connections;
}

void unregister_current_connection() {
    current_connections--;
}

void register_bytes_transferred(ssize_t bytes){
    bytes_transferred += bytes;
}

unsigned int get_bytes_transferred(){
    return bytes_transferred;
}

unsigned int get_BUFFSIZE(){
    return BUFFSIZE;
}

void set_BUFFSIZE(unsigned int newSize){
    BUFFSIZE = newSize;
}

char * get_ADMIN_TOKEN(){
    return ADMIN_TOKEN;
}

void set_ADMIN_TOKEN(char * newToken){
    strcpy(ADMIN_TOKEN, newToken);
    print_log(INFO, "NEW ADMIN TOKEN: %s", ADMIN_TOKEN);
}

bool get_dissector_state() {
    return true;
}