.PHONY=clean all
CFLAGS = -fsanitize=address -fno-omit-frame-pointer -g --std=c11 -pedantic -pedantic-errors -Wall -Wextra -Wno-unused-parameter -Wno-implicit-fallthrough -D_POSIX_C_SOURCE=200112L
LDFLAGS = -lpthread
all: socks5d ssemd
clean:	
	- rm -f *.o ./*/*.o ./*/*/*.o socks5d ssemd

COMMON =  ./utils/logger.o ./utils/util.o ./utils/buffer.o ./utils/args.o

SERVER_SOURCES = -lm ./server/server.o ./utils/selector.o ./utils/tcpServerUtil.o ./server/socks5/socks5Handler.o ./server/socketCreation.o ./server/socks5/socks5.o ./parsers/hello.o ./parsers/auth_parser.o ./parsers/pop3Parser.o ./parsers/protocolParser.o ./parsers/request.o ./server/socks5/helloState.o ./server/socks5/upState.o ./server/socks5/requestState.o ./server/socks5/connectedState.o ./server/adminFunctions/adminGets.o ./server/ssemd/ssemdHandler.o ./server/ssemd/ssemd.o
CLIENT_SOURCES = -lm ./admin/Admin.o ./admin/adminUtil.o ./admin/adminArgs.o ./admin/adminParser.o
BOMB_SOURCES = ./admin/bomb.o

logger.o: ./include/logger.h
args.o : ./include/args.h
buffer.o: ./include/buffer.h
util.o: ./include/util.h

selector.o: ./include/selector.h
tcpServerUtil.o: ./include/tcpServerUtil.h
socks5Handler.o: ./include/socks5Handler.h
socks5.o: ./include/socks5.h
hello.o: ./include/hello.h
request.o : ./include/request.h
helloState.o : ./include/helloState.h
requestState.o : ./include/requestState.h
connectedState.o : ./include/connectedState.h
adminGets.o : ./include/adminFunctions.h
ssemdHandler.o : ./include/ssemdHandler.h
protocolParser.o : ./include/protocolParser.h
ssemd.o : ./include/ssemd.h
pop3Parser.o : ./include/pop3Parser.h
upState.o : ./include/upState.h
auth_parser.o : ./include/auth_parser.h
socketCreation.o : ./include/socketCreation.h

adminUtil.o: ./admin/include/adminUtil.h
ssemdHandler.o : ./include/ssemdHandler.h
clientArgs.o: ./admin/include/adminArgs.h


socks5d: $(COMMON) $(SERVER_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o socks5d $(SERVER_SOURCES) $(COMMON)

ssemd: $(COMMON) $(CLIENT_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o ssemd $(CLIENT_SOURCES) $(COMMON)

bomb: $(COMMON) $(BOMB_SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o bomb $(BOMB_SOURCES) $(COMMON)