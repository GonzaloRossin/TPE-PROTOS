#include <stdint.h>
#include "buffer.h"
#include "logger.h"
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h> //"memset"

//socks request

enum socks_req_cmd {
    socks_req_cmd_connect   = 0x01,
    socks_req_cmd_bind      = 0x02,
    socks_req_cmd_associate = 0x03,
};

enum socks_addr_type {
    socks_req_addrtype_ipv4     = 0x01,
    socks_req_addrtype_domain   = 0x03,
    socks_req_addrtype_ipv6     = 0x04,
};

union socks_addr{
    char fqdn[0xff];
    struct sockaddr_in  ipv4;
    struct sockaddr_in6 ipv6;
};

struct request{
    enum socks_req_cmd      cmd;
    enum socks_addr_type    dest_addr_type;
    union socks_addr        dest_addr;
    in_port_t               dest_port;
};

enum request_state {
    request_version,
    request_cmd,
    request_rsv,
    request_atyp,
    request_dstaddr_fqdn,
    request_dstaddr,
    request_dstport,

    request_done,
    request_error,
    request_error_unsupported_version,
    request_error_unsupported_atyp,
};

typedef struct request_parser {
    struct request * request;
    enum request_state state;
    //cuantos bytes tenemos que leer
    uint8_t n;
    //cuantos bytes ya leímos
    uint8_t i;
} request_parser;

//6. replies

enum socks_response_status {
    status_succeeded = 0x00,
    status_general_SOCKLS_server_failure = 0x01,
    status_connection_not_allowed_by_ruleset = 0x02,
    status_network_unreachable = 0x03,
    status_host_unreachable = 0x04,
    status_connection_refused = 0x05,
    status_ttl_expired = 0x06,
    status_command_not_supported = 0x07,
    status_address_type_not_supported = 0x08,
};

//inicializa el parser
void request_parser_init(struct request_parser *p);

//entrega un byte al parser, retorna true si se llegó al final
enum request_state request_parser_feed(struct request_parser *p, const uint8_t c);

//por cada elementod el buffer llama a request_parser_feed hasta que se termine el parsing o se requieran mas bytes

enum request_state request_consume(buffer *b, struct request_parser *p, bool *errored);

//helper de hello_request
bool request_is_done(const enum request_state state, bool * errored);

//error handler
extern const char * request_error_handler(const struct request_parser * parser);

//libera recursos internos del parser
void request_close(struct request_parser * parser);

//serializa en buff la/una respuesta al request
//Retorna la cantidad de bytes ocupados del buffer, o -1 si no había espacio suficiente.
int request_marshall(buffer * buffer, const enum socks_response_status status);

//convierte a errno en socks_response_status
enum socks_response_status errno_to_socks(int e);

void free_request(struct request_parser* parser);

//se encarga de la resolución de un request
enum socks_response_status cmd_resolve(struct request * request, struct sockaddr **originaddr, socklen_t *originlen, int *domain);


