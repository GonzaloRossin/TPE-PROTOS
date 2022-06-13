#include "../../include/protocolParser.h"
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "buffer.h"

#define TYPE 0x02
#define WRONG_TYPE 0x03
#define CMD 0x11
#define SIZE 0x04

#define N(x) (sizeof(x)/sizeof((x)[0]))

int main (int argc, char * argv[]) {

    uint8_t protoTest[] = {TYPE, CMD, SIZE, 0x03, 0x01, 0x05, 0x07};

    buffer buff;
    uint8_t data[512] = {'d', 'u', 'm', 'm','y'};
    bool errored;
    buffer_init(&buff, 511, data);

    for (uint8_t * b = protoTest; b - protoTest < N(protoTest) ; b++) {
        buffer_write(&buff, *b);
    }
    struct protocol_parser pr;
    protocol_parser_init(&pr);

    enum protocol_state st = protocol_consume(&buff, &pr, &errored);
    if (st == protocol_error)
        printf("protocol Done!\n\n");

    printf("now for the wrong parsing\n\n");

    uint8_t RejectprotoTest[] = {WRONG_TYPE, CMD, SIZE, 0x03, 0x01, 0x05, 0x07};

    for (uint8_t * b = RejectprotoTest; b - RejectprotoTest < N(RejectprotoTest) ; b++) {
        buffer_write(&buff, *b);
    }

    struct protocol_parser reject_pr;
    protocol_parser_init(&reject_pr);

    st = protocol_consume(&buff, &reject_pr, &errored);
    if (st == protocol_error)
        printf("protocol wrong! bad version of socks\n");

    return 0;
}
