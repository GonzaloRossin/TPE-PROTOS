.PHONY=clean all
CFLAGS = -fsanitize=address -fno-omit-frame-pointer -g --std=c11 -pedantic -pedantic-errors -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L 
LDFLAGS = -lpthread
all: server_selector
clean:	
	- rm -f *.o server_selector

COMMON =  logger.o util.o 
SERVER_SOURCES = server_selector.o selector.o tcpServerUtil.o tcpClientUtil.o proxyHandler.o buffer.o client.o hello.o request.o helloState.o requestState.o
buffer.o: ./include/buffer.h
selector.o: ./include/selector.h
tcpServerUtil.o: ./include/tcpServerUtil.h
tcpClientUtil.o: ./include/tcpClientUtil.h
proxyHandler.o: ./include/proxyHandler.h
client.o: ./include/client.h
hello.o: ./include/hello.h
request.o : ./include/request.h
helloState.o : ./include/helloState.h
requestState.o : ./include/requestState.h


server_selector: $(COMMON) $(SERVER_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o server_selector $(SERVER_SOURCES) $(COMMON)