#include "../include/hello.h"
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "buffer.h"

#define VERSION 0x05
#define WRONG_VERSION 0x03
#define NMETHODS 0x02
#define NO_AUTHENTICATION_REQUIRED 0x00
#define USERNAME_AND_PASSWORD 0x02

#define N(x) (sizeof(x)/sizeof((x)[0]))

int main (int argc, char * argv[]) {

    uint8_t socks5Hello[] = {VERSION, NMETHODS, USERNAME_AND_PASSWORD, NO_AUTHENTICATION_REQUIRED};
    
    buffer buff;
    uint8_t data[512] = {'d', 'u', 'm', 'm','y'};
    bool errored;
    buffer_init(&buff, 511, data);

    for (uint8_t * b = socks5Hello; b - socks5Hello < N(socks5Hello) ; b++) {
        buffer_write(&buff, *b);
    }
    struct hello_parser pr;
    hello_parser_init(&pr);

    enum hello_state st = hello_consume(&buff, &pr, &errored);
    if (st == hello_done)
        printf("Hello Done!\n\n");

    printf("now for the wrong parsing\n\n");

    uint8_t Rejectsocks5Hello[] = {WRONG_VERSION, NMETHODS, NO_AUTHENTICATION_REQUIRED, USERNAME_AND_PASSWORD};

    for (uint8_t * b = Rejectsocks5Hello; b - Rejectsocks5Hello < N(Rejectsocks5Hello) ; b++) {
        buffer_write(&buff, *b);
    }

    struct hello_parser reject_pr;
    hello_parser_init(&reject_pr);

    st = hello_consume(&buff, &reject_pr, &errored);
    if (st == hello_error)
        printf("Hello wrong! bad version of socks\n");

    return 0;
}
