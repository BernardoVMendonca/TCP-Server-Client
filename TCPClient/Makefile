all: TCPCommand.o SendRecv.o client

TCPCommand.o: TCPCommand.c
	gcc -c -g TCPCommand.c

SendRecv.o: SendRecv.c
	gcc -c -g SendRecv.c

client: client.o
	gcc -o client client.o SendRecv.o TCPCommand.o -lpthread

client.o: client.c
	gcc -c -g client.c 

clean: 
	rm *.o

