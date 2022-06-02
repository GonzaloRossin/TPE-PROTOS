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
#include "proxyHandler.h"
#include "buffer.h"

#define max(n1,n2)     ((n1)>(n2) ? (n1) : (n2))

#define TRUE   1
#define FALSE  0
#define PORT 8888
#define MAX_SOCKETS 30
#define BUFFSIZE 1024
#define MAX_PENDING_CONNECTIONS   3    // un valor bajo, para realizar pruebas

/**
  Lee el datagrama del socket, obtiene info asociado con getaddrInfo y envia la respuesta
  */
void handleAddrInfo(int socket);

/**
  Maneja la actitud del master socket.
  */
void masterSocketHandler(int master_socket_size, int * master_socket, fd_set readfds, fd_set writefds, int max_clients, int * client_socket, int * remote_socket, struct buffer * bufferFromClient, struct buffer * bufferToClient);


int main(int argc , char *argv[])
{
	int opt = TRUE;
	int master_socket[2];  // IPv4 e IPv6 (si estan habilitados)
	int master_socket_size=0;
	int addrlen , client_socket[MAX_SOCKETS/2] , remote_socket[MAX_SOCKETS/2] , max_clients = MAX_SOCKETS/2 , activity, i , sd, clientSocket, remoteSocket;
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

	//initialise all client_socket[] to 0 so not checked
	memset(client_socket, 0, sizeof(client_socket));
	//initialise all remote_socket[] to 0 so not checked
	memset(remote_socket, 0, sizeof(remote_socket));

	// Agregamos un buffer de escritura asociado a cada socket, para no bloquear por escritura
	//struct buffer bufferWrite[MAX_SOCKETS];
	//memset(bufferWrite, 0, sizeof bufferWrite);

	// Agregamos un buffer de escritura asociado a cada socket, para no bloquear por escritura
	struct buffer bufferFromClient[MAX_SOCKETS];
	memset(bufferFromClient, 0, sizeof bufferFromClient);

	// Agregamos un buffer de escritura asociado a cada socket, para no bloquear por escritura
	struct buffer bufferToClient[MAX_SOCKETS];
	memset(bufferToClient, 0, sizeof bufferToClient);



	// TODO adaptar setupTCPServerSocket para que cree socket para IPv4 e IPv6 y ademas soporte opciones (y asi no repetor codigo)
	// socket para IPv4 y para IPv6 (si estan disponibles)
	///////////////////////////////////////////////////////////// IPv4
	
	if( (master_socket[master_socket_size] = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
	{
		log(ERROR, "socket IPv4 failed");
	} else {
		//set master socket to allow multiple connections , this is just a good habit, it will work without this
		if( setsockopt(master_socket[master_socket_size], SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
		{
			log(ERROR, "set IPv4 socket options failed");
		}

		//type of socket created
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons( PORT );

		// bind the socket to localhost port 8888
		if (bind(master_socket[master_socket_size], (struct sockaddr *)&address, sizeof(address))<0) 
		{
			log(ERROR, "bind for IPv4 failed");
			close(master_socket[master_socket_size]);
		}
		else {
			if (listen(master_socket[0], MAX_PENDING_CONNECTIONS) < 0)
			{
				log(ERROR, "listen on IPv4 socket failes");
				close(master_socket[master_socket_size]);
			} else {
				log(DEBUG, "Waiting for TCP IPv4 connections on socket %d\n", master_socket[master_socket_size]);
				master_socket_size++;
			}
		}
	}
	///////////////////////////////////////////////////////////// IPv6
	struct sockaddr_in6 server6addr;
	if ((master_socket[master_socket_size] = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
	{
		log(ERROR, "socket IPv6 failed");
	} else {
		if (setsockopt(master_socket[master_socket_size], SOL_SOCKET, SO_REUSEADDR, (char *)&opt,sizeof(opt)) < 0)
		{
			log(ERROR, "set IPv6 socket options failed");
		}
		memset(&server6addr, 0, sizeof(server6addr));
		server6addr.sin6_family = AF_INET6;
		server6addr.sin6_port   = htons(PORT);
		server6addr.sin6_addr   = in6addr_any;
		if (bind(master_socket[master_socket_size], (struct sockaddr *)&server6addr,sizeof(server6addr)) < 0)
		{
			log(ERROR, "bind for IPv6 failed");
			close(master_socket[master_socket_size]);
		} else {
			if (listen(master_socket[master_socket_size], MAX_PENDING_CONNECTIONS) < 0)
			{
				log(ERROR, "listen on IPv6 failed");
				close(master_socket[master_socket_size]);
			} else {
				log(DEBUG, "Waiting for TCP IPv6 connections on socket %d\n", master_socket[master_socket_size]);
				master_socket_size++;
			}
		}
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
			// socket descriptor
			clientSocket = client_socket[i];
			remoteSocket = remote_socket[i];


			// if valid socket descriptor then add to read list, and also its corresponding remote socket
			if(clientSocket > 0){
				FD_SET( clientSocket , &readfds);
				FD_SET( remoteSocket , &readfds);
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
		masterSocketHandler(master_socket_size, master_socket, readfds, writefds, max_clients, client_socket, remote_socket, bufferFromClient, bufferToClient);

		//reads from sockets
		for (i = 0; i < max_clients; i++) 
		{
			//sd = client_socket[i];
			clientSocket = client_socket[i];
			remoteSocket = remote_socket[i];
			size_t nbytes;

			//read from client
			if (FD_ISSET( clientSocket , &readfds)) 
			{
				log(DEBUG, "reading client %d on socket %d", i, clientSocket);
				//Check if it was for closing , and also read the incoming message
				if ((valread = read( clientSocket , &bufferFromClient[i].data_array , BUFFSIZE)) <= 0)
				{
					//Somebody disconnected , get his details and print
					getpeername(clientSocket , (struct sockaddr*)&address , (socklen_t*)&addrlen);
					log(INFO, "Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

					//Close the socket and mark as 0 in list for reuse
					close( clientSocket );
					client_socket[i] = 0;

					FD_CLR(clientSocket, &writefds);
					// Limpiamos el buffer asociado, para que no lo "herede" otra sesión
					buffer_reset(&bufferFromClient[i]);
				} 
				else {
					log(DEBUG, "Received %zu bytes from socket %d\n", valread, clientSocket);
					// activamos el socket para escribir en remote
					// ya se almacena en el buffer con la funcion read de arriba
					FD_SET(remoteSocket, &writefds);
					buffer_write_adv(&bufferFromClient[i], valread);
				}
			}

			//read from remote
			if (FD_ISSET(remoteSocket , &readfds)) 
			{
				log(DEBUG, "reading remote of client %d on socket %d", i, remoteSocket);
				//Check if it was for closing , and also read the incoming message
				if ((valread = read( remoteSocket , &bufferToClient[i].data_array , BUFFSIZE)) <= 0)
				{
					//Somebody disconnected , get his details and print
					getpeername(remoteSocket , (struct sockaddr*)&address , (socklen_t*)&addrlen);
					log(INFO, "Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

					//Close the socket and mark as 0 in list for reuse
					close( remoteSocket );
					remote_socket[i] = 0;

					FD_CLR(remoteSocket, &writefds);
					// Limpiamos el buffer asociado, para que no lo "herede" otra sesión
					buffer_reset(&bufferToClient[i]);
				} 
				else {
					log(DEBUG, "Received %zu bytes from (remote) socket %d\n", valread, remoteSocket);
					// activamos el socket para escritura
					// ya almacena en el buffer de salida la funcion read de arriba
					FD_SET(clientSocket, &writefds);
					buffer_write_adv(&bufferToClient[i], valread);
				}
			}
		}

		//write to sockets
		for(i =0; i < max_clients; i++) {
			clientSocket = client_socket[i];
			remoteSocket = remote_socket[i];

			//write to client
			if (FD_ISSET(clientSocket, &writefds)) {
				log(DEBUG, "remote socket %d wants to write to its client socket %d", remoteSocket, clientSocket);
				handleWrite(clientSocket, &bufferToClient[i], &writefds);
			}
			//write to remote
			if (FD_ISSET(remoteSocket, &writefds)) {
				//remote socket wants to write, write it to client
				log(DEBUG, "trying to send client content to his remote socket");
				handleWrite(remoteSocket, &bufferFromClient[i], &writefds);
			}
		}
	}

	return 0;
}


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

void masterSocketHandler(int master_socket_size, int * master_socket, fd_set readfds, fd_set writefds, int max_clients, int * client_socket, int * remote_socket, struct buffer * bufferFromClient, struct buffer * bufferToClient){
		int new_socket = 0;
		int new_remote_socket = 0;
		for (int sdMaster=0; sdMaster < master_socket_size; sdMaster++) {
			int mSock = master_socket[sdMaster];
			if (FD_ISSET(mSock, &readfds)) 
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
					if( client_socket[i] == 0 )
					{
						client_socket[i] = new_socket;
						remote_socket[i] = new_remote_socket;

						log(DEBUG, "Adding client to list of sockets as %d\n" , i);
						log(DEBUG, "Adding remote socket to client %d in socket %d\n" , i, new_remote_socket);

								//init buffer fromClient of client i
						uint8_t * data = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
						memset(data, 0, sizeof(uint8_t) * BUFFSIZE);
						buffer_init(&bufferFromClient[i], BUFFSIZE, data);
					

								//init buffer toClient of client i
						uint8_t * data_2 = (uint8_t *)malloc(sizeof(uint8_t) * BUFFSIZE);
						memset(data_2, 0, sizeof(uint8_t) * BUFFSIZE);
						buffer_init(&bufferToClient[i], BUFFSIZE, data_2);

						break;
					}
				}
			}
		}
}
