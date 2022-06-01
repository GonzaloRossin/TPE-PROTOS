#include "proxyHandler.h"

#define BUFFSIZE 1024
long valread;

struct proxyBuffer {
	char * buffer;
	size_t len;     // longitud del buffer
	size_t from;    // desde donde falta escribir
};

char buffer[BUFFSIZE + 1];  //data buffer of 1K

int handleProxyAddr(){
	char *server = "142.250.79.110"; //argv[1];     // First arg: server name IP address 
	char *echoString = "hola"; //argv[2]; // Second arg: string to echo

	// Third arg server port
	char * port = "80";//argv[3];

	// Create a reliable, stream socket using TCP
	int sock = tcpClientSocket(server, port);
	if (sock < 0) {
		log(FATAL, "socket() failed")
	}
	log(DEBUG, "new socket is %d", sock);
	return sock;
}

void readFromProxy(int remoteSocket, int clientSocket){
    if ((valread = read( remoteSocket , buffer, BUFFSIZE)) <= 0) {
        /*
        //Somebody disconnected , get his details and print
        getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
        log(INFO, "Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

        //Close the socket and mark as 0 in list for reuse
        close( sd );
        client_socket[i] = 0;

        FD_CLR(sd, &writefds);
        // Limpiamos el buffer asociado, para que no lo "herede" otra sesión
        clear(bufferWrite + i);
        */
        log(INFO, "Host disconnected");
    } 
    else {
        log(DEBUG, "Received %zu bytes from socket %d\n", valread, remoteSocket);
        // activamos el socket para escritura y almacenamos en el buffer de salida
        //FD_SET(sd, &writefds);

        // Tal vez ya habia datos en el buffer
        // TODO: validar realloc != NULL

        log(DEBUG, "%s", buffer);

        size_t bytesSent = send(clientSocket, buffer, valread,  MSG_DONTWAIT); 
        log(DEBUG, "bytesSent %d, valread %d", bytesSent, valread);

        /*
        bufferWrite[i].buffer = realloc(bufferWrite[i].buffer, bufferWrite[i].len + valread);
        memcpy(bufferWrite[i].buffer + bufferWrite[i].len, buffer, valread);
        bufferWrite[i].len += valread;
        */
    }
}