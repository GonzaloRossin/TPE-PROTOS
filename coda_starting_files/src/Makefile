.PHONY=clean all
COMPILER=gcc
CFLAGS = -Wall -fsanitize=address -g
all: clean main
clean:	
	- rm -f *.o  main

COMMON =  
main:      
	$(COMPILER) $(CFLAGS) -o main main.c selector.c$(COMMON) 