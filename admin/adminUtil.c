#include "./include/adminUtil.h"
#define MAX_ADDR_BUFFER 128


int tcpClientSocket(const char *host, const char *service) {
	char addrBuffer[MAX_ADDR_BUFFER];
	struct addrinfo addrCriteria;                   // Criteria for address match
	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
	addrCriteria.ai_family = AF_UNSPEC;             // v4 or v6 is OK
	addrCriteria.ai_socktype = SOCK_STREAM;         // Only streaming sockets
	addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

	// Get address(es)
	struct addrinfo *servAddr; // Holder for returned list of server addrs
	// int rtnVal = getaddrinfo(host, service, &addrCriteria, &servAddr); //es bloqueante, ojo, hacerlo en un hijo/thread
	int rtnVal = getaddrinfo("127.0.0.1", "8889", &addrCriteria, &servAddr);

	if (rtnVal != 0) {
		print_log(ERROR, "getaddrinfo() failed %s", gai_strerror(rtnVal));
		return -1;
	}

	int sock = -1;
	for (struct addrinfo *addr = servAddr; addr != NULL && sock == -1; addr = addr->ai_next) {
		// Create a reliable, stream socket using TCP
		sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		//setear socket como no bloqueante
		if (sock >= 0) {
			errno = 0;
			// Establish the connection to the server
			if ( connect(sock, addr->ai_addr, addr->ai_addrlen) != 0) { //se bloquea hasta que se conecte
				print_log(INFO, "can't connectto %s: %s", printAddressPort(addr, addrBuffer), strerror(errno));
				close(sock); 	// Socket connection failed; try next address
				sock = -1;
			}
		} else {
			print_log(DEBUG, "Can't create client socket on %s",printAddressPort(addr, addrBuffer));
		}
	}

	freeaddrinfo(servAddr); 
	return sock;
}

size_t getSize(struct ssemd_args *args){
	size_t toRet = 0;
	toRet++; //1 byte for VER
	int n=0;
	while(args->admin_token[n]!=0x00){
		n++;
	}
	toRet += n; //n bytes for TOKEN
	toRet++; //1 byte for \0 to mark end of TOKEN
	toRet++; //1 byte for TYPE
	toRet++; //1 byte for CMD
	toRet += 1; //2 bytes for SIZE
	toRet += args->size2; 
	if(args->size1 != 0x00){
		toRet+=args->size1 + 255;
	} //size1 size2 bytes for data
	return toRet;
}

