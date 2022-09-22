all: TCPCommand.o SendRecv.o server

TCPCommand.o: TCPCommand.c
	gcc -c -g TCPCommand.c

SendRecv.o: SendRecv.c
	gcc -c -g SendRecv.c

server: server.o
	gcc -o server server.o SendRecv.o TCPCommand.o -lpthread
	
server.o: server.c
	gcc -c -g server.c

clean: 
	rm *.o

