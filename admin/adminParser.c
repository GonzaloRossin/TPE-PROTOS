#include "./include/adminUtil.h"

extern void admin_parser_init(struct admin_parser * adminParser){
    adminParser->number = 0;
	adminParser->size = 0;
	adminParser->state = read_status;
    adminParser->dataPointer = 0;
}

extern enum admin_state admin_parser_feed(struct admin_parser * adminParser, const uint8_t byte){
    switch (adminParser->state){

        case read_status:
            if(byte == SSEMD_RESPONSE){
                adminParser->status = SSEMD_RESPONSE;
				adminParser->state = read_response_code;
            } else if (byte == SSEMD_ERROR){
                adminParser->status = SSEMD_ERROR;
				adminParser->state = read_error_code;
            } else {
				adminParser->state = read_error;
			}
            break;
        
        case read_response_code:
            if(byte == SSEMD_RESPONSE_OK){
                adminParser->response_code = SSEMD_RESPONSE_OK;
                adminParser->state = read_done;
            } else if(byte == SSEMD_RESPONSE_LIST){
                adminParser->response_code = SSEMD_RESPONSE_LIST;
                adminParser->state = read_size1;
            } else if(byte == SSEMD_RESPONSE_INT){
                adminParser->response_code = SSEMD_RESPONSE_INT;
                adminParser->state = read_size1;
            } else if(byte == SSEMD_RESPONSE_BOOL){
                adminParser->response_code = SSEMD_RESPONSE_BOOL;
                adminParser->state = read_size1;
            } else {
                adminParser->state = read_error;
            }
            break;

        case read_size1:
            if(byte != 0x00){
                adminParser->size += 256 * (uint8_t) byte;
            }
            adminParser->size1 = byte;
            adminParser->state = read_size2;
            break;

        case read_size2:
            if(byte == 0x00){
                if(adminParser->size == 0){
                    adminParser->state = read_error; //it should contain data but doesnt
                } else {
                    adminParser->data = (unsigned char *)malloc(sizeof(uint8_t) * adminParser->size);
                    adminParser->state = read_data;
                }
            } else {
                adminParser->size += (uint8_t) byte;
                adminParser->data = (unsigned char *)malloc(sizeof(uint8_t) * adminParser->size);
                adminParser->state = read_data;
            }
            adminParser->size2 = byte;
            break;

        case read_data:
            adminParser->data[adminParser->dataPointer++] = byte;
            if(adminParser->dataPointer == adminParser->size){
                adminParser->state = read_done;
            }
            break;

        // case read_error_code:


        case read_error:
        case read_done:
            //nada que hacer
            break;
        
        default:
            //log(ERROR, "unknown state %d", parser->state);
            abort();
            break;
    }
    return adminParser->state;
}

extern bool admin_is_done(const enum admin_state state, bool * errored){
    bool ret;
    switch (state)
        {
        case read_error:
            if (0 != errored){
                *errored = true;
            }
            // no break;
        case read_done:
            ret = true;
            break;

        default:
            ret = false;
            break;
        }
    return ret;
}

extern const char * admin_error_handler(const struct admin_parser * parser){
    char* ret;
    switch (parser->state)
        {
        case read_error:
            ret = "unsupported version";
            break;
        
        default:
            ret = "";
            break;
        }
    return ret;
}

extern void admin_parser_close(struct admin_parser * parser){
    //no hay nada que liberar
}

extern enum admin_state admin_consume(buffer * buffer, struct admin_parser * parser, bool *errored){
    enum admin_state st = parser->state;

    while(buffer_can_read(buffer)) {
        const uint8_t c = buffer_read(buffer);
        st = admin_parser_feed(parser, c);
        if(admin_is_done(st, errored)){ //falta el interior de esto
            break;
        }
    }
    return st;
}

void printInt(struct admin_parser * adminParser){
    int j;
    double power;
    unsigned int number;
    for(j=0; j<4; j++){
        power = pow(256, 4 - j -1);
        number = adminParser->data[j];
        adminParser->number +=  number * power;
    }
    print_log(INFO, "number: %u", adminParser->number);
}

void printList(struct admin_parser * adminParser){
    int i;
    printf("\n\t");
    for(i=0; i<adminParser->size; i++){
        printf("%c", adminParser->data[i]);
        if(adminParser->data[i] == '\0'){
            printf("\n\t");
        }
    }
    printf("\n");
}

void printBool(struct admin_parser * adminParser){
    if(adminParser->data[0] == SSEMD_TRUE){
        printf("turned ON\n");
    } else if(adminParser->data[0] == SSEMD_FALSE){
        printf("turned OFF\n");
    } else {
        print_log(ERROR, "Reading boolean response");
    }
}
