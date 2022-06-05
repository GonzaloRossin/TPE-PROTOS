.PHONY=clean all
COMPILER=gcc
CFLAGS = -Wall -fsanitize=address -g -lpthread
all: clean tcpEchoClient server udpEchoClient
clean:	
	- rm -f *.o  tcpEchoclient server udpEchoClient

COMMON =  logger.c util.c
addrinfo:      
	$(COMPILER) $(CFLAGS) -o addrinfo addrinfo.c $(COMMON) 
tcpEchoClient:      
	$(COMPILER) $(CFLAGS) -o tcpEchoClient tcpEchoClient.c tcpClientUtil.c $(COMMON)
tcpEchoServer:      
	$(COMPILER) $(CFLAGS) -o tcpEchoServer tcpEchoServer.c tcpServerUtil.c $(COMMON)
server:      
	$(COMPILER) $(CFLAGS) -o server server.c tcpServerUtil.c tcpClientUtil.c proxyHandler.c buffer.c client.c $(COMMON)
server_selector:      
	$(COMPILER) $(CFLAGS) -o server_selector server_selector.c selector.c tcpServerUtil.c tcpClientUtil.c proxyHandler.c buffer.c client.c  $(COMMON)
udpEchoServer:      
	$(COMPILER) $(CFLAGS) -o udpEchoServer udpEchoServer.c $(COMMON)
udpEchoClient:
	$(COMPILER) $(CFLAGS) -o udpEchoClient udpEchoClient.c $(COMMON)