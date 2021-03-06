#include <stdint.h>
#include "./../include/hello.h"


extern void hello_parser_init(struct hello_parser * parser){
    memset(parser, 0, sizeof(struct hello_parser));
    parser->auth = NULL;
}

extern enum hello_state hello_parser_feed(struct hello_parser * parser, const uint8_t byte){
    switch (parser->state){

        case hello_version:
            if(byte == 0x05){
                parser->state = hello_nmethods;
            } else {
                parser->state = hello_error;
            }
            break;
        
        case hello_nmethods:
            if (byte == 0) {
                parser->state = hello_error;
            }
            parser->nauth = byte;
            parser->auth = malloc(byte);
            parser->bytes_to_read = byte;
            parser->state = hello_methods;
            break;

        case hello_methods:
            if (parser->bytes_to_read) {
                parser->auth[parser->nauth - parser->bytes_to_read] = byte;
                parser->bytes_to_read--;
                if (parser->bytes_to_read == 0){
                    parser->state = hello_done;
                }
            }
            break;
        
        case hello_done:
        case hello_error:
            //nada que hacer
            break;
        
        default:
            //log(ERROR, "unknown state %d", parser->state);
            abort();
            break;
    }
    return parser->state;
}

extern bool hello_is_done(const enum hello_state state, bool * errored){
    bool ret;
    switch (state)
        {
        case hello_error:
            if (0 != errored){
                *errored = true;
            }
            // no break;
        case hello_done:
            ret = true;
            break;

        default:
            ret = false;
            break;
        }
    return ret;
}

extern const char * hello_error_handler(const struct hello_parser * parser){
    char* ret;
    switch (parser->state)
        {
        case hello_error:
            ret = "unsupported version";
            break;
        
        default:
            ret = "";
            break;
        }
    return ret;
}

extern void hello_parser_close(struct hello_parser * parser){
    free(parser->auth);
}

extern enum hello_state hello_consume(buffer * buffer, struct hello_parser * parser, bool *errored){
    enum hello_state st = parser->state;

    while(buffer_can_read(buffer)) {
        const uint8_t c = buffer_read(buffer);
        st = hello_parser_feed(parser, c);
        if(hello_is_done(st, errored)){
            break;
        }
    }
    return st;
}

extern int hello_marshall(buffer * buffer, const uint8_t method){
    size_t n;
    uint8_t * buff = buffer_write_ptr(buffer, &n);
    if(n<2) {
        return -1;
    }
    buff[0] = 0x05;
    buff[1] = method;
    buffer_write_adv(buffer, 2);
    return 2;
}



