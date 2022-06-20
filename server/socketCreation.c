#include "../include/socketCreation.h"

int create_master_sockets(int * master_socket, int * master_socket_6, struct socks5args * args) {
    // Address for socket binding
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(args->socks_addr);
    address.sin_port = htons(args->socks_port);

    // Generating the IPv6 address structure
    struct in6_addr in6addr;
    inet_pton(AF_INET6, args->socks_addr6, &in6addr);

    // Address for IPv6 socket binding
    struct sockaddr_in6 address6;
    memset(&address6, 0, sizeof(address6));
    address6.sin6_family = AF_INET6;
    address6.sin6_addr = in6addr;
    address6.sin6_port = htons(args->socks_port);

    *master_socket = create_master_socket_4(&address);
    *master_socket_6 = create_master_socket_6(&address6);
    return 1;
}

int create_master_socket_4(struct sockaddr_in * addr) {
    // Creating the server socket to listen
    int master_socket = socket(AF_INET, SOCK_STREAM, 0);

    int opt = true;

    if (master_socket <= 0) {
        printf("socket failed");
        //log(FATAL, "socket failed");
        exit(EXIT_FAILURE);
    }

    // Setting the master socket to allow multiple connections, not required, just good habit
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        print_log(ERROR, "Failure setting setting master socket\n");
        exit(EXIT_FAILURE);
    }

    // Binding the socket to localhost:1080
    if (bind(master_socket, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
        print_log(ERROR, "Failure binding master socket\n");
        exit(EXIT_FAILURE);
    }

    // Checking if the socket is able to listen
    if (listen(master_socket, MAX_PENDING_CONNECTIONS) < 0) {
        print_log(ERROR, "Failure listening master socket\n");
        exit(EXIT_FAILURE);
    }

    return master_socket;
}

int create_master_socket_6(struct sockaddr_in6 * addr) {
    int master_socket6 = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
    if (master_socket6 <= 0) {
        printf("socket6 failed");
        //log(FATAL, "socket failed");
        exit(EXIT_FAILURE);
    }

    int yes = 1;
    int opt = true;

    if (setsockopt(master_socket6, SOL_IPV6, IPV6_V6ONLY, (void *)&yes, sizeof(yes)) < 0) {
        print_log(ERROR, "Failed to set IPV6_V6ONLY\n");
        exit(EXIT_FAILURE);
    }

    // Setting the master socket to allow multiple connections, not required, just good habit
    if (setsockopt(master_socket6, SOL_IPV6, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0) {
        print_log(ERROR, "Failure setting setting IPv6 master socket\n");
        exit(EXIT_FAILURE);
    }

    // Binding the socket to localhost:1080
    if (bind(master_socket6, (struct sockaddr *)addr, sizeof(*addr)) < 0) {
        print_log(ERROR, "Failure binding IPv6 master socket\n");
        exit(EXIT_FAILURE);
    }

    // Checking if the socket is able to listen
    if (listen(master_socket6, MAX_PENDING_CONNECTIONS) < 0) {
        print_log(ERROR, "Failure listening IPv6 master socket\n");
        exit(EXIT_FAILURE);
    }
    return master_socket6;
}