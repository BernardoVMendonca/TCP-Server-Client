#ifndef SENDRECV_H
#define SENDRECV_H

#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <unistd.h>

enum command
{
   COMMAND_LIST,
   COMMAND_DOWNLOAD,
   COMMAND_STAT,
   COMMAND_FINISH
};

typedef struct fileInfo
{
   int socket;
   char *file;
   int n;
   int nTotal;
} fileInfo;

void sendLInt(long int number, int socket);
long int recvLInt(int socket);

void sendInt(int number, int socket); // ok
int recvInt(int socket);              // ok

void sendDouble(double number, int socket); // ok
double recvDouble(int socket);              // ok

void sendString(char *string, int socket); // ok
char *recvString(int socket);              // ok

void *sendFile(void *fileA, int fd, char *addr); // ok

#endif