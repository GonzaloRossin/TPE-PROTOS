.PHONY=clean all
CFLAGS = -fsanitize=address -fno-omit-frame-pointer -g --std=c11 -pedantic -pedantic-errors -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L 
LDFLAGS = -lpthread
all: server_selector
clean:	
	- rm -f *.o server_selector

COMMON =  logger.o util.o 
SERVER_SOURCES = server_selector.o selector.o tcpServerUtil.o tcpClientUtil.o proxyHandler.o buffer.o client.o
buffer.o: buffer.h
selector.o: selector.h
tcpServerUtil.o: tcpServerUtil.h
tcpClientUtil.o: tcpClientUtil.h
proxyHandler.o: proxyHandler.h
buffer.o: buffer.h
client.o: client.h


server_selector: $(COMMON) $(SERVER_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o server_selector $(SERVER_SOURCES) $(COMMON)