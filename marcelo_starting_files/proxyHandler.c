#include "proxyHandler.h"

#define BUFFSIZE 1024
long valread;

struct proxyBuffer {
	char * buffer;
	size_t len;     // longitud del buffer
	size_t from;    // desde donde falta escribir
};

char buffer_hardcode[BUFFSIZE + 1];  //data buffer of 1K

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

void readFromProxy(int remoteSocket, int clientSocket, fd_set * writefds) {
    if ((valread = read( remoteSocket , buffer_hardcode, BUFFSIZE)) <= 0) {
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

        //log(DEBUG, "%s", buffer_hardcode);

        size_t bytesSent = send(clientSocket, buffer_hardcode, valread,  MSG_DONTWAIT); 
        log(DEBUG, "bytesSent %zu, valread %ld", bytesSent, valread);
        FD_CLR(remoteSocket, writefds);

        /*
        bufferWrite[i].buffer = realloc(bufferWrite[i].buffer, bufferWrite[i].len + valread);
        memcpy(bufferWrite[i].buffer + bufferWrite[i].len, buffer, valread);
        bufferWrite[i].len += valread;
        */
    }
}

// Hay algo para escribir?
// Si está listo para escribir, escribimos. El problema es que a pesar de tener buffer para poder
// escribir, tal vez no sea suficiente. Por ejemplo podría tener 100 bytes libres en el buffer de
// salida, pero le pido que mande 1000 bytes.Por lo que tenemos que hacer un send no bloqueante,
// verificando la cantidad de bytes que pudo consumir TCP.
void handleWrite(int socket, struct buffer * buffer, fd_set * writefds) {
	size_t bytesToSend = buffer->write - buffer->read;
	log(DEBUG, "bytesToSend %zu", bytesToSend);
	if (bytesToSend > 0) {  // Puede estar listo para enviar, pero no tenemos nada para enviar
		log(INFO, "Trying to send %zu bytes to socket %d\n", bytesToSend, socket);
		log(DEBUG, "sending:\n%s", buffer->data);
		size_t bytesSent = send(socket, buffer->data, bytesToSend,  MSG_DONTWAIT); 
		log(INFO, "Sent %zu bytes\n", bytesSent);

		if ( bytesSent < 0) {
			// Esto no deberia pasar ya que el socket estaba listo para escritura
			// TODO: manejar el error
			log(FATAL, "Error sending to socket %d", socket);
		} else {
			size_t bytesLeft = bytesSent - bytesToSend;
			buffer_read_adv(buffer, bytesSent);


			// Si se pudieron mandar todos los bytes limpiamos el buffer y sacamos el fd para el select
			
			if ( bytesLeft == 0) {
				//buffer_reset(buffer);
				FD_CLR(socket, writefds);
			} else {
				//buffer->write += bytesSent;
			}
		}
	}
}