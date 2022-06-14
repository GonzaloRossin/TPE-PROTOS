#ifndef HELLO_H
#define HELLO_H

#include "buffer.h"
#include "logger.h"


// static uint8_t METHOD_NO_AUTHENTICATION_REQUIRED = 0x00;
// static uint8_t METHOD_NO_ACCEPTABLE_METHODS = 0xFF;
// static uint8_t METHOD_USERNAME_AND_PASSWORD = 0x02;



/*
+------+-------------+------------+
| VER  |   NMETHODS  |   METHODS  |
+------+-------------+------------+
|  1   |      1      |  1 to 255  |
+------+-------------+------------+


*/

enum hello_state {
    hello_version,
    hello_nmethods,
    hello_methods,
    hello_done,
    hello_error,
};

//estado interno del parser
typedef struct hello_parser {
    //invocado cada vez que se presenta un nuevo método
    void(*on_authentication_method)
        (struct hello_parser *parser, const uint8_t method);
    
    //para que el usuario del parser almacene sus datos
    void *data;

    enum hello_state state;

    uint8_t remaining_methods;
} hello_parser;

void hello_parser_init (struct hello_parser *parser);

//consume un byte, es llamado por hello_consume
enum hello_state hello_parser_feed (struct hello_parser *parser, uint8_t byte);

//por cada elemento del buffer, se llama a hello_parser_feed hasta que se complete el parseo
// y avisa en que estado terminó
enum hello_state hello_consume(buffer * b, struct hello_parser *parser, bool * errored); //errored parametro de salida

//helper de hello_consume
bool hello_is_done(const enum hello_state state, bool * errored);

//error handler
extern const char * hello_error_handler(const struct hello_parser * parser);

//libera recursos internos del parser
void hello_parser_close(struct hello_parser * parser);

//serializa en buff la/una respuesta al hello
//Retorna la cantidad de bytes ocupados del buffer, o -1 si no había espacio suficiente.
int hello_marshall(buffer * buffer, const uint8_t method);


#endif