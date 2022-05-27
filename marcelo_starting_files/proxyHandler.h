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

//Se encarga de devolver el socket a quien el proxy debe mandar
//crea un nuevo socket para una nueva página, o devuelve el socket existente para una conexión ya establecida)
int handleProxyAddr();

void readFromProxy(int socket);