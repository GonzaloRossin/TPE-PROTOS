#include "../include/socketCreation.h"

int create_master_sockets(int * master_socket, int * master_socket_6) {
    // // Address for socket binding
    // struct sockaddr_in address;
    // memset(&address, 0, sizeof(address));
    // address.sin_family = AF_INET;
    // address.sin_addr.s_addr = inet_addr(options->socks_addr);
    // address.sin_port = htons(options->socks_port);

    // // Generating the IPv6 address structure
    // struct in6_addr in6addr;
    // inet_pton(AF_INET6, options->socks_addr_6, &in6addr);

    // // Address for IPv6 socket binding
    // struct sockaddr_in6 address6;
    // memset(&address6, 0, sizeof(address6));
    // address6.sin6_family = AF_INET6;
    // address6.sin6_addr = in6addr;
    // address6.sin6_port = htons(options->socks_port);

    // *master_socket = create_master_socket_4(&address);
    // *master_socket_6 = create_master_socket_6(&address6);
    return 1;
}