//#include "../hello.h"
#include "../include/hello.h"
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "buffer.h"

#define N(x) (sizeof(x)/sizeof((x)[0]))

int main (int argc, char * argv[]) {

    uint8_t sampleIPv4Address[] = {0x05, 0x02, 0x00, 0x02};
    
    buffer buff;
    uint8_t data[512] = {'h', 'o', 'l', 'a'};
    bool errored;
    buffer_init(&buff, 511, data);

    for (uint8_t * b = sampleIPv4Address; b - sampleIPv4Address < N(sampleIPv4Address) ; b++) {
        buffer_write(&buff, *b);
    }
    struct hello_parser pr;
    hello_parser_init(&pr);
    enum hello_state st = hello_consume(&buff, &pr, &errored);
    if (st == hello_done)
        printf("Hello Done!\n");
}
