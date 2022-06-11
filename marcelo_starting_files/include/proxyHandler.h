#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
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


#define BUFFSIZE 4096


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
void socks5_write(struct selector_key *key);
void socks5_close(struct selector_key *key);
void socks5_block(struct selector_key *key);

void hello_read(struct selector_key *key);
void hello_write(struct selector_key *key);

void request_read(struct selector_key *key);
enum client_state request_connect(struct selector_key *key);
void request_connecting(struct selector_key *key);
//serializa en buff la/una respuesta al request
//Retorna la cantidad de bytes ocupados del buffer, o -1 si no había espacio suficiente.
int request_marshall(struct socks5 * currClient);
void request_write(struct selector_key *key);
void request_resolve(struct selector_key *key);

void write_connected_state(struct selector_key *key);
void read_connected_state(struct selector_key *key);
void change_state(struct socks5 * currClient, enum client_state state);
void * request_resolv_blocking(void *data);
enum client_state process_request(struct selector_key *key);

enum socks_addr_type family_to_socks_addr_type(int family);
