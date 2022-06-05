//parser del request de SOCKS5
#include "request.h"

static void remaining_set(struct request_parser * p, const int n){
    p->i = 0;
    p->n = n;
}

static int remaining_is_done(struct request_parser * p) {
    return p->i >= p->n;
}

//////////////////////

static enum request_state version(const uint8_t c, struct request_parser * p){
    enum request_state next;
    switch(c) {
        case 0x05:
            next = request_cmd;
            break;
        default:
            next = request_error_unsupported_version;
            break;
    }
    return next;
}

static enum request_state cmd(const uint8_t c, struct request_parser * p) {
    p->request->cmd = c;

    return request_rsv;
}

static enum request_state rsv(const uint8_t c, struct request_parser * p){
    return request_atyp;
}

static enum request_state atyp(const uint8_t c, struct request_parser * p){
    enum request_state next;

    p->request->dest_addr_type = c;
    switch (p->request->dest_addr_type) {
        case socks_req_addrtype_ipv4:
            remaining_set(p, 4);
            memset(&(p->request->dest_addr.ipv4), 0, sizeof(p->request->dest_addr.ipv4));
            p->request->dest_addr.ipv4.sin_family = AF_INET;
            next = request_dstaddr;
            break;
        case socks_req_addrtype_ipv6:
            remaining_set(p, 16);
            memset(&(p->request->dest_addr.ipv6), 0, sizeof(p->request->dest_addr.ipv6));
            p->request->dest_addr.ipv6.sin6_family = AF_INET6;
            next = request_dstaddr;
            break;
        case socks_req_addrtype_domain:
            next = request_dstaddr_fqdn;
            break;
        default:
            next = request_error_unsupported_atyp;
            break;
    }
    return next;
}

static enum request_state dstaddr_fqdn(const uint8_t c, struct request_parser * p){
    remaining_set(p, c);
    p->request->dest_addr.fqdn[p->n - 1] = 0;

    return request_dstaddr;
}

static enum request_state dstaddr(const uint8_t c, struct request_parser * p){
    enum request_state next;

    switch (p->request->dest_addr_type) {
         case socks_req_addrtype_ipv4:
            ((uint8_t *)&(p->request->dest_addr.ipv4.sin_addr))[p->i++] = c;//creo que termina así el renglón
            break;
        case socks_req_addrtype_ipv6:
            ((uint8_t *)&(p->request->dest_addr.ipv6.sin6_addr))[p->i++] = c;//creo que termina así el renglón
            break;
        case socks_req_addrtype_domain:
            p->request->dest_addr.fqdn[p->i++] = c;
            break;
    }
    if (remaining_is_done(p)) {
        remaining_set(p, 2);
        p->request->dest_port = 0;
        next = request_dstport;
    } else {
        next = request_dstaddr;
    }

    return next;
}

static enum request_state dstport(const uint8_t c, struct request_parser * p){ //ojo acá con el ntohs o htons, port es network order osea big endian, talvez hay que cambiarlo
    enum request_state  next;
    *(((uint8_t *) &(p->request->dest_port)) + p->i) = c;
    p->i++;
    next = request_dstport;
    if(p->i >= p->n) {
        next = request_done;
    }
    return next;
}

extern void request_parser_init (struct request_parser * p) {
    p->state = request_version;
    p->request = calloc(1,sizeof(struct request));
    //memset(p->request,0,sizeof(struct request));
}

extern enum request_state request_consume(buffer * buffer, struct request_parser * parser, bool *errored){
    enum request_state st = parser->state;

    while(buffer_can_read(buffer)) {
        const uint8_t c = buffer_read(buffer);
        st = request_parser_feed(parser, c);
        if(request_is_done(st, errored)){
            break;
        }
    }
    return st;
}
void free_request(struct request_parser* parser){
    if(parser != NULL){
        free(parser->request);
    }
    return;
}
extern bool request_is_done(const enum request_state state, bool * errored){
    bool ret;
    switch (state)
        {
        case request_error:
            if (0 != errored){
                *errored = true;
            }
            // no break;
        case request_done:
            ret = true;
            break;

        default:
            ret = false;
            break;
        }
    return ret;
}
extern enum request_state request_parser_feed(struct request_parser * p, const uint8_t c){
    enum request_state next;

    switch(p->state) {
        case request_version:
            next = version(c, p);
            break;
        case request_cmd:
            next = cmd(c, p);
            break;
        case request_rsv:
            next = rsv(c, p);
            break;
        case request_atyp:
            next = atyp(c, p);
            break;
        case request_dstaddr_fqdn:
            next = dstaddr_fqdn(c, p);
            break;
        case request_dstaddr:
            next = dstaddr(c, p);
            break;
        case request_dstport:
            next = dstport(c, p);
            break;
        case request_done:
        case request_error:
        case request_error_unsupported_version:
        case request_error_unsupported_atyp:
            next = p->state;
            break;
        default:
            next = request_error;
            break;
    }
    return p->state = next;
}
#include <errno.h>

enum socks_response_status errno_to_socks(const int e){
    enum socks_response_status ret = status_general_SOCKLS_server_failure;
    switch (e)
    {
    case 0:
        ret = status_succeeded;
        break;
    case ECONNREFUSED:
        ret = status_connection_refused;
        break;
    case EHOSTUNREACH:
        ret = status_host_unreachable;
        break;
    case ETIMEDOUT:
        ret = status_ttl_expired;
        break;
    }
    return ret;
}
//hay mas acá abajo que se ve en el video (tipo 2:50:00) pero no doy más bro
