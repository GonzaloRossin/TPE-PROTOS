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
#include "./include/logger.h"
#include "./include/tcpServerUtil.h"
#include "./include/tcpClientUtil.h"
#include "./include/proxyHandler.h"
#include "./include/buffer.h"
#include "./include/client.h"

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
void masterSocketHandler(int master_socket_size, int * master_socket, fd_set readfds, fd_set writefds, int max_clients, struct client * clients);


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


	// Limpiamos el conjunto de escritura
	FD_ZERO(&writefds);
	while(TRUE) 
	{
		//clear the socket set
		FD_ZERO(&readfds);

		//add masters sockets to set
		for (int sdMaster=0; sdMaster < master_socket_size; sdMaster++){
			FD_SET(master_socket[sdMaster], &readfds);
			max_sd = master_socket[sdMaster];
		}

		// add child sockets to set
		for ( i = 0 ; i < max_clients ; i++) 
		{
			clientSocket = clients[i].client_socket;
			remoteSocket = clients[i].remote_socket;

			// if valid socket descriptor
			if(clientSocket > 0){
				//and can write in buffer, subscribe socket for reading
				if( buffer_can_write(&(clients[i].bufferFromClient)) ){
					FD_SET( clientSocket , &readfds);
				}
				if( buffer_can_write(&(clients[i].bufferFromRemote)) ){
					FD_SET( remoteSocket , &readfds);
				}
				//and can read buffer, subscribe socket for writing
				if( buffer_can_read(&(clients[i].bufferFromClient)) ){
					FD_SET( remoteSocket , &writefds);
				}
				if( buffer_can_read(&(clients[i].bufferFromRemote)) ){
					FD_SET( clientSocket , &writefds);
				}
			}
			// highest file descriptor number, need it for the select function
			if(clientSocket > max_sd)
				max_sd = clientSocket;
			if(remoteSocket > max_sd)
				max_sd = remoteSocket;
		}

		//wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
		activity = select( max_sd + 1 , &readfds , &writefds , NULL , NULL);

		log(DEBUG, "select has something...");	

		if ((activity < 0) && (errno!=EINTR)) 
		{
			log(ERROR, "select error, errno=%d",errno);
			continue;
		}

		//If something happened on the TCP master socket , then its an incoming connection
		//masterSocketHandler(master_socket_size, master_socket, readfds, writefds, max_clients, client_socket, remote_socket, bufferFromClient, bufferFromRemote);
		masterSocketHandler(master_socket_size, master_socket, readfds, writefds, max_clients, clients);

		size_t nbytes;

		//reads from sockets
		for (i = 0; i < max_clients; i++) {
			//sd = client_socket[i];
			clientSocket = clients[i].client_socket;
			remoteSocket = clients[i].remote_socket;

			//read from client
			if (FD_ISSET( clientSocket , &readfds)) 
			{
				log(DEBUG, "reading client %d on socket %d", i, clientSocket);
				//Check if it was for closing , and also read the incoming message
				nbytes = clients[i].bufferFromClient.limit - clients[i].bufferFromClient.write;
				log(DEBUG, "available bytes to write in bufferFromClient: %zu", nbytes)
				if ((valread = read( clientSocket , clients[i].bufferFromClient.data, nbytes)) <= 0) //hace write en el buffer
				{
					//Somebody disconnected , get his details and print
					getpeername(clientSocket , (struct sockaddr*)&address , (socklen_t*)&addrlen);
					log(INFO, "Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

					removeClient(&clients[i]);
					FD_CLR(clientSocket, &writefds);
				} 
				else {
					log(DEBUG, "Received %zu bytes from socket %d\n", valread, clientSocket);
					// ya se almacena en el buffer con la funcion read de arriba
					buffer_write_adv(&clients[i].bufferFromClient, valread);
				}
			}

			//read from remote
			if (FD_ISSET(remoteSocket , &readfds)) 
			{
				log(DEBUG, "reading remote of client %d on socket %d", i, remoteSocket);
				nbytes = clients[i].bufferFromRemote.limit - clients[i].bufferFromRemote.write;
				log(DEBUG, "available bytes to write in bufferFromRemote: %zu", nbytes)
				//Check if it was for closing , and also read the incoming message
				if ((valread = read( remoteSocket , clients[i].bufferFromRemote.data , nbytes)) <= 0) //escribe en el buffer
				{
					//Somebody disconnected , get his details and print
					getpeername(remoteSocket , (struct sockaddr*)&address , (socklen_t*)&addrlen);
					log(INFO, "Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

					removeClient(clients[i]);
					FD_CLR(remoteSocket, &writefds);
				} 
				else {
					log(DEBUG, "Received %zu bytes from (remote) socket %d:\n", valread, remoteSocket);
					// ya almacena en el buffer de salida la funcion read de arriba
					buffer_write_adv(&clients[i].bufferFromRemote, valread);
				}
			}
		}

		//write to sockets
		for(i =0; i < max_clients; i++) {
			clientSocket = clients[i].client_socket;
			remoteSocket = clients[i].remote_socket;

			//write to client
			if (FD_ISSET(clientSocket, &writefds)) {
				log(DEBUG, "remote socket %d wants to write to his client socket %d", remoteSocket, clientSocket);
				handleWrite(clientSocket, &clients[i].bufferFromRemote, &writefds);
			}
			//write to remote
			if (FD_ISSET(remoteSocket, &writefds)) {
				log(DEBUG, "trying to send client content to his remote socket");
				handleWrite(remoteSocket, &clients[i].bufferFromClient, &writefds);
			}
		}
	}
	return 0;
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


void masterSocketHandler(int master_socket_size, int * master_socket, fd_set readfds, fd_set writefds, int max_clients, struct client * clients){
		int new_socket = 0;
		int new_remote_socket = 0;
		for (int sdMaster=0; sdMaster < master_socket_size; sdMaster++) {
			int mSock = master_socket[sdMaster];
			if (FD_ISSET(mSock, &readfds)) //esto creo que no va
			{
				if ((new_socket = acceptTCPConnection(mSock)) < 0) //open new client socket
				{
					log(ERROR, "Accept error on master socket %d", mSock);
					continue;
				}

				if ((new_remote_socket = handleProxyAddr()) < 0) //open new remote socket
				{
					log(ERROR, "Accept error on creating new remote socket %d", new_remote_socket);
					continue;
				}

				// add new socket to array of sockets
				int i;
				for (i = 0; i < max_clients; i++) 
				{
					// if position is empty
					if( clients[i].isAvailable )
					{
						new_client(&clients[i], new_socket, BUFFSIZE);
						set_client_remote(&clients[i], new_remote_socket, BUFFSIZE);

						log(DEBUG, "Adding client %d in socket %d\n" , i, new_socket);
						log(DEBUG, "Adding remote socket to client %d in socket %d\n" , i, new_remote_socket);

						break;
					}
				}
			}
		}
}
