.PHONY=clean all
CFLAGS = -fsanitize=address -fno-omit-frame-pointer -g --std=c11 -pedantic -pedantic-errors -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L 
LDFLAGS = -lpthread
all: server_selector server_client
clean:	
	- rm -f *.o server_selector server_client

COMMON =  logger.o util.o buffer.o args.o

SERVER_SOURCES = server_selector.o selector.o tcpServerUtil.o  socks5Handler.o client.o hello.o request.o helloState.o requestState.o connectedState.o
CLIENT_SOURCES = ./admin/Admin.o ./admin/adminUtil.o ./admin/adminArgs.o

buffer.o: ./include/buffer.h
selector.o: ./include/selector.h
tcpServerUtil.o: ./include/tcpServerUtil.h
socks5Handler.o: ./include/socks5Handler.h
client.o: ./include/client.h
hello.o: ./include/hello.h
request.o : ./include/request.h
helloState.o : ./include/helloState.h
requestState.o : ./include/requestState.h
connectedState.o : ./include/connectedState.h

adminUtil.o: ./admin/include/adminUtil.h
ssemdHandler.o : ./include/ssemdHandler.h
args.o : ./include/args.h
clientArgs.o: ./admin/include/adminArgs.h

server_selector: $(COMMON) $(SERVER_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o server_selector $(SERVER_SOURCES) $(COMMON)

server_client: $(COMMON) $(CLIENT_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o server_client $(CLIENT_SOURCES) $(COMMON)