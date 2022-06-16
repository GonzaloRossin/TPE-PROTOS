#include "../include/protocolParser.h"
#define TOKEN_SIZE 25


bool on_size_authentication_method(struct protocol_parser* p, uint8_t byte){
    switch(p->data->type){
        case 0x01:{
            if(byte != 0x00){
                return true;
            }else{
                return false;
            }
        }
        case 0x02:{
            switch (p->data->CMD)
            {
            case 0x03:
            case 0x04:
            case 0x07:
            case 0x08:
                if(byte != 0x00){
                    return true;
                }
                return false;
                break;
            
            default:
                break;
            }
        }
    }
    return false;
}

extern void protocol_parser_init(struct protocol_parser * parser){
    parser->state = protocol_version;
    parser->data =(payload*) calloc(1,sizeof (struct payload));
    parser->on_size_authentication_method = &(on_size_authentication_method);
    parser->data->data_len=0;
}

extern enum protocol_state protocol_parser_feed(struct protocol_parser * parser, const uint8_t byte){
    switch (parser->state){
        case protocol_version:
            if( byte == 0x01){
                parser->data->version = byte;
                parser->state = protocol_token;
                parser->data->token = malloc(TOKEN_SIZE* sizeof(uint8_t));
                parser->token_index = 0;
                break;
            }
        case protocol_token:
            if(parser->token_index % TOKEN_SIZE == 0){
                     parser->data->token = realloc(parser->data->token,parser->token_index + TOKEN_SIZE);
            }
            if(byte == 0x00){
                parser->data->token[parser->token_index] = byte;
                parser->data->token = realloc(parser->data->token,parser->token_index+1);
                parser->data->token_len = parser->token_index;
                parser->state = protocol_type;
            }else{
                parser->data->token[parser->token_index++] = byte;
            }
            /*parser->data->token = "j";
            parser->data->token_len = 1;
            if(byte == 0x00) {
                parser->state = protocol_type;
            }*/
        break;
        case protocol_type:
            if(byte == 0x01){
                parser->data->type= byte;
                parser->state = protocol_cmd_get;
            } else if(byte == 0x02){
                parser->data->type= byte;
                parser->state = protocol_cmd_edit;
            }else{
                parser->state =protocol_error;
            }
            break;

        case protocol_cmd_get:
            if(byte <= 0x08){
                parser->data->CMD = byte;
                parser->state = protocol_size1;
            }
            else{
                parser->state = protocol_error;
            }
            break;

        case protocol_cmd_edit:
            if(byte <= 0x08){
                parser->data->CMD = byte;
                parser->state = protocol_size1;
            }
            else{
                parser->state = protocol_error;
            }
            break;

        case protocol_size1:
            if(parser->on_size_authentication_method != NULL ) {
                if(parser->on_size_authentication_method(parser, byte)){//devuelve falso si el size no matchea con el comando
                    parser->state = protocol_error;
                    break;
                }
            }
            parser->size1 = byte;
            // memset(&(parser->data->data), 0, parser->size);
            // parser->data->data = calloc(1,parser->size);
            parser->state = protocol_size2;
            break;
        
        case protocol_size2:
            parser->size2 = byte;
            int size=parser->size2;

            if(parser->size1 != 0x00){
                size += 255 + parser->size1;
            }
            if(size > 0){
                parser->data->data = calloc(1, sizeof(uint8_t) * size);
                parser->state = protocol_data;
            } else {
                parser->state = protocol_done;
            }
            break;

        case protocol_data:
            size = parser->size2;
            if(parser->size1 != 0x00){
                size += 255 + parser->size1;
            }

            parser->data->data[parser->data->data_len] = byte;
            parser->data->data_len++;
            if(parser->data->data_len == size){
                parser->state = protocol_done;
            }

        case protocol_done:
        case protocol_error:
            //nada que hacer
            break;

        default:
            //log(ERROR, "unknown state %d", parser->state);
            abort();
            break;
    }
    return parser->state;
}

extern bool protocol_is_done(const enum protocol_state state, bool * errored){
    bool ret;
    switch (state)
    {
        case protocol_error:
            if (0 != errored){
                *errored = true;
            }
            // no break;
        case protocol_done:
            ret = true;
            break;

        default:
            ret = false;
            break;
    }
    return ret;
}

extern const char * protocol_error_handler(const struct protocol_parser * parser){
    char* ret;
    switch (parser->state)
    {
        case protocol_error:
            ret = "unsupported version";
            break;

        default:
            ret = "";
            break;
    }
    return ret;
}

extern void protocol_parser_close(struct protocol_parser * parser){
    free(parser->data->data);
    free(parser->data);
}

extern enum protocol_state protocol_consume(buffer * buffer, struct protocol_parser * parser, bool *errored){
    enum protocol_state st = parser->state;

    while(buffer_can_read(buffer)) {
        const uint8_t c = buffer_read(buffer);
        st = protocol_parser_feed(parser, c);
        if(protocol_is_done(st, errored)){
            break;
        }
    }
    return st;
}

extern int protocol_marshall(buffer * buffer, const uint8_t method){
    size_t n;
    uint8_t * buff = buffer_write_ptr(buffer, &n);
    if(n<2){
        return -1;
    }
    buff[0] = 0x05;
    buff[1] = method;
    buffer_write_adv(buffer, 2);
    return 2;
}