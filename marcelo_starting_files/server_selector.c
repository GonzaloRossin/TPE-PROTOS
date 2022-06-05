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
#include "logger.h"
#include "tcpServerUtil.h"
#include "tcpClientUtil.h"
#include "proxyHandler.h"
#include "buffer.h"
#include "client.h"
#include "selector.h"

static bool done = false;

static void
sigterm_handler(const int signal) {
    printf("signal %d, cleaning up and exiting\n",signal);
    done = true;
}

#define max(n1,n2)     ((n1)>(n2) ? (n1) : (n2))

#define TRUE   1
#define FALSE  0
#define PORT "8888"
#define MAX_SOCKETS 30
#define BUFFSIZE 4096
#define MAX_PENDING_CONNECTIONS   3    // un valor bajo, para realizar pruebas



/**
  Lee el datagrama del socket, obtiene info asociado con getaddrInfo y envia la respuesta
  */
void handleAddrInfo(int socket);

/**
  Maneja la actividad del master socket.
  */
void masterSocketHandler(struct selector_key *key);


int main(int argc , char *argv[])
{
	int master_socket[2];  // IPv4 e IPv6 (si estan habilitados)
	int master_socket_size=0;
	int addrlen , max_clients = MAX_SOCKETS/2 , activity, i , clientSocket, remoteSocket;
	struct client clients[max_clients];
	long valread;
	int max_sd;
	struct sockaddr_in address;

	struct sockaddr_storage clntAddr; // Client address
	socklen_t clntAddrLen = sizeof(clntAddr);

	const char       *err_msg = NULL;
	selector_status   ss      = SELECTOR_SUCCESS;
    fd_selector selector      = NULL;

	//char buffer[BUFFSIZE + 1];  //data buffer of 1K

	//set of socket descriptors
	fd_set readfds;

	// y tambien los flags para writes
	fd_set writefds;

	//initialise all clients to 0
	memset(clients, 0, sizeof(clients));
	for(i=0; i<max_clients; i++){
		clients[i].isAvailable = true;
	}

	// socket para IPv4 y para IPv6 (si estan disponibles)
	///////////////////////////////////////////////////////////// IPv4
	if ((master_socket[master_socket_size] = setupTCPServerSocket(PORT, AF_INET)) < 0) {
		log(ERROR, "socket IPv4 failed");
	} else {
		log(DEBUG, "Waiting for TCP IPv4 connections on socket %d\n", master_socket[master_socket_size]);
		master_socket_size++;
	}

	if ((master_socket[master_socket_size] = setupTCPServerSocket(PORT, AF_INET6)) < 0) {
		log(ERROR, "socket IPv6 failed");
	} else {
		log(DEBUG, "Waiting for TCP IPv6 connections on socket %d\n", master_socket[master_socket_size]);
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
        .handle_read       = masterSocketHandler,
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

    //socksv5_pool_destroy();

	for (int i = 0; i < master_socket_size; i++) {
		if(master_socket[i] >= 0) {
        	close(master_socket[i]);
    	}
	}
    
    return ret;
}


//era del echo UDP pero talvez nos sirva
void handleAddrInfo(int socket) {
	// En el datagrama viene el nombre a resolver
	// Se le devuelve la informacion asociada

	char buffer[BUFFSIZE];
	unsigned int len, n;

	struct sockaddr_in clntAddr;

	// Es bloqueante, deberian invocar a esta funcion solo si hay algo disponible en el socket    
	n = recvfrom(socket, buffer, BUFFSIZE, 0, ( struct sockaddr *) &clntAddr, &len);
	if ( buffer[n-1] == '\n') // Por si lo estan probando con netcat, en modo interactivo
		n--;
	buffer[n] = '\0';
	log(DEBUG, "UDP received:%s", buffer );
	// TODO: parsear lo recibido para obtener nombre, puerto, etc. Asumimos viene solo el nombre

	// Especificamos solo SOCK_STREAM para que no duplique las respuestas
	struct addrinfo addrCriteria;                   // Criteria for address match
	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
	addrCriteria.ai_family = AF_UNSPEC;             // Any address family
	addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
	addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol


	// Armamos el datagrama con las direcciones de respuesta, separadas por \r\n
	// TODO: hacer una concatenacion segura
	// TODO: modificar la funcion printAddressInfo usada en sockets bloqueantes para que sirva
	//       tanto si se quiere obtener solo la direccion o la direccion mas el puerto
	char bufferOut[BUFFSIZE];
	bufferOut[0] = '\0';

	struct addrinfo *addrList;
	int rtnVal = getaddrinfo(buffer, NULL, &addrCriteria, &addrList);
	if (rtnVal != 0) {
		log(ERROR, "getaddrinfo() failed: %d: %s", rtnVal, gai_strerror(rtnVal));
		strcat(strcpy(bufferOut,"Can't resolve "), buffer);

	} else {
		for (struct addrinfo *addr = addrList; addr != NULL; addr = addr->ai_next) {
			struct sockaddr *address = addr->ai_addr;
			char addrBuffer[INET6_ADDRSTRLEN];

			void *numericAddress = NULL;
			switch (address->sa_family) {
				case AF_INET:
					numericAddress = &((struct sockaddr_in *) address)->sin_addr;
					break;
				case AF_INET6:
					numericAddress = &((struct sockaddr_in6 *) address)->sin6_addr;
					break;
			}
			if ( numericAddress == NULL) {
				strcat(bufferOut, "[Unknown Type]");
			} else {
				// Convert binary to printable address
				if (inet_ntop(address->sa_family, numericAddress, addrBuffer, sizeof(addrBuffer)) == NULL)
					strcat(bufferOut, "[invalid address]");
				else {
					strcat(bufferOut, addrBuffer);
				}
			}
			strcat(bufferOut, "\r\n");
		}
		freeaddrinfo(addrList);
	}

	// Enviamos respuesta (el sendto no bloquea)
	sendto(socket, bufferOut, strlen(bufferOut), 0, (const struct sockaddr *) &clntAddr, len);

	log(DEBUG, "UDP sent:%s", bufferOut );
}


void masterSocketHandler(struct selector_key *key) {
	struct client *cli;
	
	const int new_client_socket = acceptTCPConnection(key->fd);

	selector_fd_set_nio(new_client_socket);

	const int new_remote_socket = handleProxyAddr();

	if ((new_remote_socket < 0)) {//open new remote socket
		log(ERROR, "Accept error on creating new remote socket %d", new_remote_socket);
	}

	// add new socket to array of sockets
	int i;
	struct clients_data * cli_data = (struct clients_data *)key->data;
	struct client * clis = cli_data->clients;

	for (i = 0; i < cli_data->clients_size; i++) 
	{
		// if position is empty
		if(clis[i].isAvailable )
		{
			new_client(&clis[i], new_client_socket, BUFFSIZE);
			set_client_remote(&clis[i], new_remote_socket, BUFFSIZE);

			const struct fd_handler socksv5 = {
				.handle_read       = NULL,
				.handle_write      = NULL,
				.handle_close      = NULL, // nada que liberar
    		};
			
			int ss = selector_register(key->s, new_client_socket, &socksv5, OP_READ, &clis[i]);

			log(DEBUG, "Adding client %d in socket %d\n" , i, new_client_socket);
			log(DEBUG, "Adding remote socket to client %d in socket %d\n" , i, new_remote_socket);

			break;
		}
	}
}
