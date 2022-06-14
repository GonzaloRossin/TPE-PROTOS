/*Connected state
After the initial negotiation is complete, the Client is expected to send a message with the following format:

Base connected-state package structure: for Client requests
+--------+-------+--------+---------+
|  TYPE  |  CMD  |  SIZE  |  DATA   |
+--------+-------+--------+---------+
|   1    |   1   |  0-2   | 0-65535 |
+-- -----+-------+--------+---------+

Base connected-state package structure: for Server responses
+--------+-------+--------+---------+
|  TYPE  |  CODE |  SIZE  |  DATA   |
+--------+-------+--------+---------+
|   1    |   1   |  0-2   | 0-65535 |
+-- -----+-------+--------+---------+

The TYPE field is to identify which TYPE of command is requested.
The values currently defined for TYPE are:
- X'01' : GET. The current Client Tequest corresponds to a 'GET' command type. The Client will ignore these messages.
- X'02' : EDIT. The current Client Request corresponds to an 'EDIT' command type. The Client will ignore these messages.
- X'AA' : RESPONSE. The current package corresponds to a Server response. The Server will ignore these messages.
- X'FF' : ERROR. The current package corresponds to a Server ERROR response. The Server will ignore these messages.
*/

#ifndef TPE_PROTOS_PROTOCOLPARSER_H
#define TPE_PROTOS_PROTOCOLPARSER_H

#include <stdint.h>
#include "buffer.h"
#include <stdlib.h>
#include <stdbool.h>

enum protocol_state {
    protocol_type,
    protocol_cmd,
    protocol_cmd_get,
    protocol_cmd_edit,
    protocol_size,
    protocol_page,
    protocol_data,
    protocol_done,
    protocol_error,
};
typedef struct payload{
    uint8_t type;
    uint8_t CMD;
    int data_len;
    uint8_t* data;
}payload;
//estado interno del parser
typedef struct protocol_parser {

    //invocado cada vez que se presenta un nuevo método
   bool(*on_size_authentication_method)
            (struct protocol_parser *parser, const uint8_t byte);

    payload* data;
    uint8_t size;
    uint8_t page;
    uint8_t i;
    enum protocol_state state;

    uint8_t remaining_methods;
} protocol_parser;

void protocol_parser_init (struct protocol_parser *parser);

//consume un byte, es llamado por hello_consume
enum protocol_state protocol_parser_feed (struct protocol_parser *parser, uint8_t byte);

//por cada elemento del buffer, se llama a hello_parser_feed hasta que se complete el parseo
// y avisa en que estado terminó
enum protocol_state protocol_consume(buffer * b, struct protocol_parser *parser, bool * errored); //errored parametro de salida

//helper de hello_consume
bool protocol_is_done(const enum protocol_state state, bool * errored);

//error handler
extern const char * protocol_error_handler(const struct protocol_parser * parser);

//libera recursos internos del parser
void protocol_parser_close(struct protocol_parser * parser);

//serializa en buff la/una respuesta al hello
//Retorna la cantidad de bytes ocupados del buffer, o -1 si no había espacio suficiente.
int protocol_marshall(buffer * buffer, const uint8_t method);

#endif //TPE_PROTOS_PROTOCOLPARSER_H
