#include "../../include/upState.h"

void up_read_init(struct socks5 * currClient) {
    struct userpass_st * up_s = &currClient->client.userpass;

    up_s->r = currClient->bufferFromClient;
    up_s->w = currClient->bufferFromRemote;
    up_s->parser = (up_req_parser)calloc(1, sizeof(struct up_req_parser));
    up_req_parser_init(up_s->parser);
}   

void up_read_close(struct socks5 * currClient) {
	free_up_req_parser(currClient->client.userpass.parser);
}

void up_read(struct selector_key *key) {
    struct socks5 * currClient = (struct socks5 *)key->data;
    struct userpass_st *up_s = &((struct socks5 *)key->data)->client.userpass;

    bool error = false, auth_valid = false;
    uint8_t *ptr;
    size_t count;
    ssize_t n;

    ptr = buffer_write_ptr(up_s->r, &count);
    n = recv(key->fd, ptr, count, 0);
    if (n > 0) {
        buffer_write_adv(up_s->r, n);

        const enum up_req_state st = up_consume_message(up_s->r, up_s->parser, &error);
        if (up_done_parsing(st, &error) && !error) {
            selector_set_interest_key(key, OP_WRITE);
            userpass_process(up_s, &auth_valid);
            if(auth_valid) {
                currClient->username = up_s->user;
            }
        }
    }
}
void userpass_process(struct userpass_st *up_s, bool * auth_valid) {
    uint8_t * uid = up_s->parser->uid;
    uint8_t * pw = up_s->parser->pw;
    uint8_t uid_l = up_s->parser->uidLen;

    *auth_valid = validate_user_proxy(uid, pw);
    
    if(*auth_valid){            
        up_s->user = malloc(uid_l + 1);
        memcpy(up_s->user, uid, uid_l);
        up_s->user[uid_l] = 0x00;
    }           
            

    // Serialize the auth result in the write buffer for the response.
    if(-1 == up_marshall(up_s->w, *auth_valid ? AUTH_SUCCESS : AUTH_FAILURE)){

    }
}

bool validate_user_proxy(uint8_t *uid, uint8_t *pw) {

}
