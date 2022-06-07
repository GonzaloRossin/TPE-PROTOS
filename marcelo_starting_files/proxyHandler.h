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
#include "logger.h"
#include "tcpServerUtil.h"
#include "tcpClientUtil.h"
#include "buffer.h"
#include "client.h"
#include "selector.h"

//Se encarga de devolver el socket a quien el proxy debe mandar
//crea un nuevo socket para una nueva página, o devuelve el socket existente para una conexión ya establecida)
int handleProxyAddr();

/**
  Maneja la actividad del master socket.
  */
void masterSocketHandler(struct selector_key *key);

void readFromProxy(int remoteSocket, int clientSocket, fd_set * writefds);

/**
  Se encarga de escribir la respuesta faltante en forma no bloqueante, retorna bytesLeft To Send
  */
int handleWrite(int socket, struct buffer * buffer);

void socks5_read(struct selector_key *key);

//read y write handlers del client (si éste quiere leer o escribir)
void socks5_active_read_client(struct selector_key *key);
void socks5_active_write_client(struct selector_key *key);

//read y write handlers del remote (si éste quiere leer o escribir)
void socks5_active_read_remote(struct selector_key *key);
void socks5_active_write_remote(struct selector_key *key);

