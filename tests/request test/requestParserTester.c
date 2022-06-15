#include "../include/request.h"
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define VERSION 0x05
#define CMD 0x01
#define RSV 0x00
#define ATYP_IPV4 0x01
#define ATYP_DNAME 0x03
#define ATYP_IPV6 0x04
#define USERNAME_AND_PASSWORD 0x02

#define N(x) (sizeof(x)/sizeof((x)[0]))

int main (int argc, char * argv[]) {

    uint8_t sampleIPv4Address[] = {VERSION, CMD, RSV, ATYP_IPV4, 0x04, 0x02, 0x01, 0x01, 0x77, 0x00};
    uint8_t sampleInvalidIPv4Address[] = {VERSION, CMD, RSV, ATYP_IPV6, 0x04, 0x02, 0x01, 0x01, 0x77, 0x00};
    uint8_t sampleDNAMEAddress[] = {VERSION, CMD, RSV, ATYP_DNAME, 0x0E, 0x77, 0x77, 0x77, 0x2e, 0x67, 0x6f, 0x6f, 0x67, 0x6c, 0x65, 0x2e, 0x63, 0x6f, 0x6d, 0x77, 0x00};
    uint8_t sampleIPv6Address[] = {VERSION, CMD, RSV, ATYP_IPV6, 0x04, 0x02, 0x01, 0x01, 0x14, 0x12, 0x11, 0x11, 0x24, 0x22, 0x21, 0x21, 0x34, 0x32, 0x31, 0x31, 0x77, 0x00};
    
    buffer buff;
    uint8_t data[512] = {'d', 'u', 'm', 'm', 'y'};
    bool errored;
    buffer_init(&buff, 511, data);

    for (uint8_t * b = sampleIPv4Address; b - sampleIPv4Address < N(sampleIPv4Address) ; b++) {
        buffer_write(&buff, *b);
    }
    struct request_parser pr;
    request_parser_init(&pr);

    enum request_state st = request_consume(&buff, &pr, &errored);
    if (st = request_done)
        printf("IPv4 Done!\n");

    free_request(&pr);
    //------------------------------------------------------------------
    buffer_init(&buff, 511, data);

    for (uint8_t * b = sampleIPv6Address; b - sampleIPv6Address < N(sampleIPv6Address) ; b++) {
        buffer_write(&buff, *b);
    }
    request_parser_init(&pr);
    st = request_consume(&buff, &pr, &errored);
    if (st = request_done)
        printf("IPv6 Done!\n");

    free_request(&pr);
    //--------------------------------------------------------------------
    buffer_init(&buff, 511, data);

    for (uint8_t * b = sampleDNAMEAddress; b - sampleDNAMEAddress < N(sampleDNAMEAddress) ; b++) {
        buffer_write(&buff, *b);
    }
    request_parser_init(&pr);
    st = request_consume(&buff, &pr, &errored);
    if (st = request_done)
        printf("DNAME Done!\n");
    free_request(&pr);
    //--------------------------------------------------------------------------
    printf("checking invalid response ipv4\n");
    buffer_init(&buff, 511, data);

    for (uint8_t * b = sampleInvalidIPv4Address; b - sampleInvalidIPv4Address < N(sampleInvalidIPv4Address) ; b++) {
        buffer_write(&buff, *b);
    }
    request_parser_init(&pr);
    st = request_consume(&buff, &pr, &errored);
    if (st = request_error)
        printf("invalid IP for ATYP!\n");
    free_request(&pr);

    return 0;
}
