CC = clang
#CC = gcc

CFLAGS = -Wall -Wextra -pthread -DDEBUG=2 -Weverything 
#-std=c99  #-coverage #-g

all: client 

#TODO Ajouter la biblio doxy

udpTools.o: udpTools.c
	    $(CC) -c udpTools.c -o udpTools.o $(CFLAGS)

client.o: client.c 
	    $(CC) -c client.c -o client.o $(CFLAGS)

client: client.o udpTools.o
	    $(CC) client.o udpTools.o -o client $(CFLAGS)

clean:
	rm -f *.o client 
