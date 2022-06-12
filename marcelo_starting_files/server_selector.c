#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <unistd.h>   
#include <arpa/inet.h>    
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> 
#include <signal.h>
#include "./include/logger.h"
#include "./include/tcpServerUtil.h"
#include "./include/tcpClientUtil.h"
#include "./include/socks5Handler.h"
#include "./include/buffer.h"
#include "./include/selector.h"
#include "./include/args.h"

static bool done = false;

static void
sigterm_handler(const int signal) {
    printf("signal %d, cleaning up and exiting\n",signal);
    done = true;
}

#define max(n1,n2)     ((n1)>(n2) ? (n1) : (n2))

#define TRUE   1
#define FALSE  0
//#define PORT "8888"
//#define ADMIN_PORT "8889"
#define MAX_SOCKETS 1000
#define MAX_PENDING_CONNECTIONS   10   // un valor bajo, para realizar pruebas



/**
  Lee el datagrama del socket, obtiene info asociado con getaddrInfo y envia la respuesta
  */
void handleAddrInfo(int socket);


int main(int argc , char *argv[])
{
    //pongo esto primero así no corre nada de más si pone mal los args
    struct socks5args * args = (struct socks5args *)malloc(sizeof(struct socks5args));
	parse_args(argc, argv, args);

	int master_socket[4];  // IPv4 e IPv6 (si estan habilitados)
	int master_socket_size=0;
	int max_clients = MAX_SOCKETS/2 , i;
	struct socks5 * clients;

	const char       *err_msg = NULL;
	selector_status   ss      = SELECTOR_SUCCESS;
    fd_selector selector      = NULL;

	//char buffer[BUFFSIZE + 1];  //data buffer of 1K

	//initialise all clients to 0
	// memset(clients, 0, sizeof(clients));
    clients = (struct socks5 *)calloc(max_clients, sizeof(struct socks5));
	for(i=0; i<max_clients; i++){
		clients[i].isAvailable = true;
	}

    char PORT[6];
    char ADMIN_PORT[6];
    sprintf(PORT, "%d", args->socks_port);
    sprintf(ADMIN_PORT, "%d", args->mng_port);
	// master sockets para IPv4 y para IPv6 (si estan disponibles)
	/////////////////////////////////////////////////////////////
	if ((master_socket[master_socket_size] = setupTCPServerSocket(PORT, AF_INET)) < 0) {
		print_log(ERROR, "socket IPv4 failed");
	} else {
		print_log(DEBUG, "\nWaiting for TCP IPv4 connections on socket %d", master_socket[master_socket_size]);
		master_socket_size++;
	}

	if ((master_socket[master_socket_size] = setupTCPServerSocket(PORT, AF_INET6)) < 0) {
		print_log(ERROR, "socket IPv6 failed");
	} else {
		print_log(DEBUG, "Waiting for TCP IPv6 connections on socket %d\n", master_socket[master_socket_size]);
		master_socket_size++;
	}

    // Master sockets para atender admins IPv4 y para IPv6 (si estan disponibles)
	/////////////////////////////////////////////////////////////
    if ((master_socket[master_socket_size] = setupTCPServerSocket(ADMIN_PORT, AF_INET)) < 0) {
		print_log(ERROR, "socket IPv4 failed");
	} else {
		print_log(DEBUG, "Waiting for TCP IPv4 connections on socket %d FOR ADMIN ONLY", master_socket[master_socket_size]);
		master_socket_size++;
	}

	if ((master_socket[master_socket_size] = setupTCPServerSocket(ADMIN_PORT, AF_INET6)) < 0) {
		print_log(ERROR, "socket IPv6 failed");
	} else {
		print_log(DEBUG, "Waiting for TCP IPv6 connections on socket %d FOR ADMIN ONLY\n", master_socket[master_socket_size]);
		master_socket_size++;
	}


	signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);
	for (int i = 0; i < master_socket_size; i++) {
		if(selector_fd_set_nio(master_socket[i]) == -1) {
			err_msg = "getting server socket flags";
			goto finally;
		}
	}
    
    const struct selector_init conf = {
        .signal = SIGALRM,
        .select_timeout = {
            .tv_sec  = 10000,
            .tv_nsec = 0,
        },
    };
    if(0 != selector_init(&conf)) {
        err_msg = "initializing selector";
        goto finally;
    }

    selector = selector_new(1024);
    if(selector == NULL) {
        err_msg = "unable to create selector";
        goto finally;
    }
    const struct fd_handler socksv5 = {
        .handle_read       = masterSocks5Handler,
        .handle_write      = NULL,
        .handle_close      = NULL, // nada que liberar
    };

	struct clients_data clients_struct = {
		.clients = clients,
		.clients_size = max_clients
	};

	for (int i = 0; i < master_socket_size; i++) {
		ss = selector_register(selector, master_socket[i], &socksv5, OP_READ, &clients_struct);

		if(ss != SELECTOR_SUCCESS) {
        err_msg = "registering fd";
        goto finally;
    	}
	}
    
    for(;!done;) {
        err_msg = NULL;
        ss = selector_select(selector);
        if(ss != SELECTOR_SUCCESS) {
            err_msg = "serving";
            goto finally;
        }
    }
    if(err_msg == NULL) {
        err_msg = "closing";
    }

    int ret = 0;
finally:
    if(ss != SELECTOR_SUCCESS) {
        fprintf(stderr, "%s: %s\n", (err_msg == NULL) ? "": err_msg,
                                  ss == SELECTOR_IO
                                      ? strerror(errno)
                                      : selector_error(ss));
        ret = 2;
    } else if(err_msg) {
        perror(err_msg);
        ret = 1;
    }
    if(selector != NULL) {
        selector_destroy(selector);
    }
    selector_close();

    free(clients);
	for (int i = 0; i < master_socket_size; i++) {
		if(master_socket[i] >= 0) {
        	close(master_socket[i]);
    	}
	}
    free(args);
    
    return ret;
}

