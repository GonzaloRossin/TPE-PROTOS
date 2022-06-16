#include "../../include/adminFunctions.h"

static unsigned int historic_connections = 0;
static unsigned int current_connections = 0;
static unsigned int bytes_transferred = 0;

static unsigned int BUFFSIZE = 4096;

static char ADMIN_TOKEN[100];

static bool dissector_status = true;
static bool auth_status = true;

struct users * users = NULL;

void init_users(struct users* new_users){
    users = (struct users *)malloc(sizeof(struct users) * MAX_USERS);
    memcpy(users, new_users, sizeof(new_users));
}

struct users * get_users(){
    return users;
}

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
    return dissector_status;
}

bool set_dissector_ON(){
    dissector_status = true;
    if(dissector_status == true){
        return true;
    } else {
        return false;
    }
}

bool set_dissector_OFF(){
    dissector_status = false;
    if(dissector_status == false){
        return true;
    } else {
        return false;
    }
}

bool get_auth_state() {
    return auth_status;
}

bool set_auth_ON(){
    auth_status = true;
    if(auth_status == true){
        return true;
    } else {
        return false;
    }
}

bool set_auth_OFF(){
    auth_status = false;
    if(auth_status == false){
        return true;
    } else {
        return false;
    }
}