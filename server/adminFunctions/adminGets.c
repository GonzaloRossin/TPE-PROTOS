#include "../../include/adminFunctions.h"

static unsigned int historic_connections = 0;
static unsigned int current_connections = 0;
static unsigned int bytes_transferred = 0;

static unsigned int BUFFSIZE = 4096;
static unsigned int timeout = 1;

static char ADMIN_TOKEN[100];

static bool dissector_status = true;
static bool auth_status = true;

static struct users myUsers[MAX_USERS];

void init_users(struct users * new_users){
    for (int i = 0; i < MAX_USERS; i++) {
        if(new_users[i].name != '\0'){
            myUsers[i].name = (char *)calloc(1, sizeof(uint8_t) * 21);
            memcpy(myUsers[i].name, new_users[i].name, strlen(new_users[i].name) + 1);
        }
        if(new_users[i].pass != '\0'){
            myUsers[i].pass = (char *)calloc(1, sizeof(uint8_t) * 21);
            memcpy(myUsers[i].pass, new_users[i].pass, strlen(new_users[i].pass) + 1);
        }
    }
}

void free_users(){
    for (int i = 0; i < MAX_USERS; i++) {
        if(myUsers[i].name != '\0'){
            free(myUsers[i].name);
        }
        if(myUsers[i].pass != '\0'){
            free(myUsers[i].pass);
        }
    }
}

struct users * get_users() {
    return myUsers;
}

// unsigned int 

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

void register_bytes_transferred(unsigned int bytes){
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

unsigned int get_timeout(){
    return timeout;
}

void set_timeout(unsigned int newSize){
    timeout = newSize;
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