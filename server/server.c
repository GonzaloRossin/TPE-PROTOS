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
#include "../include/logger.h"
#include "../include/tcpServerUtil.h"
#include "../include/socks5Handler.h"
#include "../include/ssemdHandler.h"
#include "../include/buffer.h"
#include "../include/selector.h"
#include "../include/args.h"
#include "../include/ssemdHandler.h"
#include "../include/adminFunctions.h"
#include "../include/socketCreation.h"

static bool done = false;

static void
sigterm_handler(const int signal) {
    printf("signal %d, cleaning up and exiting\n",signal);
    done = true;
}

#define max(n1,n2)     ((n1)>(n2) ? (n1) : (n2))

#define TRUE   1
#define FALSE  0
#define MAX_SOCKETS 1000  // un valor bajo, para realizar pruebas
#define MAX_ADMINS 10

static const struct fd_handler socksv5 = {
    .handle_read       = masterSocks5Handler,
    .handle_write      = NULL,
    .handle_close      = NULL, // nada que liberar
};

static const struct fd_handler ssemdf = {
    .handle_read       = masterssemdHandler,
    .handle_write      = NULL,
    .handle_close      = NULL, // nada que liberar
};

/**
  Lee el datagrama del socket, obtiene info asociado con getaddrInfo y envia la respuesta
  */
void handleAddrInfo(int socket);


int main(int argc , char *argv[])
{
    //pongo esto primero así no corre nada de más si pone mal los args
    struct socks5args * args = (struct socks5args *)malloc(sizeof(struct socks5args));
	parse_args(argc, argv, args);

    char PORT[6];
    char ADMIN_PORT[6];
    sprintf(PORT, "%d", args->socks_port); //sets client port
    sprintf(ADMIN_PORT, "%d", args->mng_port); //sets admin port
    set_ADMIN_TOKEN(args->admin_token);
    init_users(args->users);
    if(! args->disectors_enabled){
        set_dissector_OFF();
    }

    int masterSocket = -1, masterSocket_6 = -1, adminSocket = -1, adminSocket_6 = -1;
	int max_clients = MAX_SOCKETS/2 , i;
	struct socks5 * clients;
    struct ssemd * admins;
    int max_admins = MAX_ADMINS;

	const char       *err_msg = NULL;
	selector_status   ss      = SELECTOR_SUCCESS;
    fd_selector selector      = NULL;

	//initialise all clients to 0
	// memset(clients, 0, sizeof(clients));
    clients = (struct socks5 *)calloc(max_clients, sizeof(struct socks5));
	for(i=0; i<max_clients; i++){
		clients[i].isAvailable = true;
	}

    //initialise Admin
    admins = (struct ssemd*)calloc(MAX_ADMINS, sizeof(struct ssemd));
    for (i = 0; i < max_admins; i++) {
        admins[i].isAvailable = true;
        admins[i].pr = NULL;
    }
    // createSocks5MasterSockets();
	// master sockets para IPv4 y para IPv6 (si estan disponibles)
	/////////////////////////////////////////////////////////////
    create_master_sockets(&masterSocket, &masterSocket_6, args);
    create_admin_sockets(&adminSocket, &adminSocket_6, args);

	signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);
    
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
    

    struct clients_data *clients_struct = (struct clients_data *)calloc(1, sizeof(struct clients_data));
	    clients_struct->clients = clients;
        clients_struct->clients_size = max_clients;

    struct admins_data *admins_struct = (struct admins_data *)calloc(1, sizeof(struct admins_data));
	    admins_struct->admins = admins;
        admins_struct->admins_size = max_admins;

    printf("Waiting for proxy connections on: \n");
    if (masterSocket > 0) {

        ss = selector_register(selector, masterSocket, &socksv5, OP_READ, clients_struct);
        if(ss != SELECTOR_SUCCESS) {
            err_msg = "registering fd";
            goto finally;
        }
        printf("IP: %s PORT: %d \n", args->socks_addr, args->socks_port);
    }
    
    if (masterSocket_6 > 0) {
        ss = selector_register(selector, masterSocket_6, &socksv5, OP_READ, clients_struct);
        if(ss != SELECTOR_SUCCESS) {
            err_msg = "registering fd";
            goto finally;
            }
        printf("IP: %s PORT: %d \n", args->socks_addr6, args->socks_port);
    }
    
    printf("Waiting for ADMIN connections on: \n");
    if (adminSocket > 0) {
        ss = selector_register(selector, adminSocket, &ssemdf, OP_READ, admins_struct);
        if(ss != SELECTOR_SUCCESS) {
            err_msg = "registering fd";
            goto finally;
        }
        printf("IP: %s PORT: %d \n", args->mng_addr, args->mng_port);
    }
    
    if (adminSocket_6 > 0) {
        ss = selector_register(selector, adminSocket_6, &ssemdf, OP_READ, admins_struct);
        if(ss != SELECTOR_SUCCESS) {
            err_msg = "registering fd";
            goto finally;
        }
        printf("IP: %s PORT: %d \n", args->mng_addr6, args->mng_port);
    }

    printf("date\t\t\tusername\tregister_type\tOrigin_IP:Origin_port\tDestination:Dest_Port\t\tstatus\n");

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


    free(admins);
    free(clients);
    close(masterSocket);
    close(masterSocket_6);
    close(adminSocket);
    close(adminSocket_6);
    free(args);
    free(clients_struct);
    free(admins_struct);

    free_users();
    
    return ret;
}

