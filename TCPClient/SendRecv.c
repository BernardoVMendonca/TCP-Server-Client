#include "SendRecv.h"

void sendLInt(long int number, int socket)
{
   send(socket, &number, sizeof(long int), 0);
}
long int recvLInt(int socket)
{
   long int Number;
   recv(socket, &Number, sizeof(Number), 0);
   return Number;
}

void sendInt(int number, int socket)
{
   send(socket, &number, sizeof(int), 0);
}
int recvInt(int socket)
{
   int Number;
   recv(socket, &Number, sizeof(Number), 0);
   return Number;
}

void sendDouble(double number, int socket)
{
   send(socket, &number, sizeof(double), 0);
}
double recvDouble(int socket)
{
   double Number;
   recv(socket, &Number, sizeof(Number), 0);
   return Number;
}

void sendString(char *string, int socket)
{
   send(socket, string, strlen(string) * sizeof(char), 0);
}

char *recvString(int socket)
{
   char *string = (char *)calloc(257, sizeof(char));
   recv(socket, string, 256 * sizeof(char), 0);
   return string;
}

void *recvFile(int socket, int fd, int n)
{
   long int partSize;
   long int recvSize = recv(socket, &partSize, sizeof(long int), 0);
   long int recvBuff = 0;

   char *buff = (char *)calloc(partSize + 1, sizeof(char));

   while (recvBuff < partSize)
   {
      recvBuff += recv(socket, buff + recvBuff, (partSize - recvBuff) * sizeof(char), 0);
   }

   long int offset;
   long int recvOffset = recv(socket, &offset, sizeof(long int), 0);
   int writeCheck = pwrite(fd, buff, partSize, offset);
   if (writeCheck == -1)
   {
      printf("FILE WRITE ERROR:%s\n", strerror(errno));
      return NULL;
   }
   printf("part:%d bytesRcv:%ld bytesWrite:%d\n", n, recvBuff, writeCheck);
   free(buff);
   return NULL;
}