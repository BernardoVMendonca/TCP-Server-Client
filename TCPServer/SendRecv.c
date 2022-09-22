#include "SendRecv.h"
#include <sys/stat.h>
#include <sys/mman.h>

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

void *sendFile(void *file, int fd, char *addr)
{
   fileInfo *fa = (fileInfo *)file;

   struct stat sb;
   if (stat(fa->file, &sb) == -1)
   {
      perror("stat");
      exit(EXIT_FAILURE);
   }

   long int size = sb.st_size,
            sizeLeft = size % fa->nTotal,
            sizePart = size / fa->nTotal;
   long int offset = sizePart * (fa->n - 1);
   if (fa->n == fa->nTotal)
      sizePart += sizeLeft;
   long int sendSize = send(fa->socket, &sizePart, sizeof(long int), 0);
   if (sendSize == -1)
   {
      printf("SEND SIZE ERROR:%s\n", strerror(errno));
      return NULL;
   }
   long int sendAddr = send(fa->socket, addr + offset, sizePart * sizeof(char), 0);
   if (sendAddr == -1)
   {
      printf("SEND ADDR ERROR:%s\n", strerror(errno));
      return NULL;
   }
   long int sendOffset = send(fa->socket, &offset, sizeof(long int), 0);
   if (sendOffset == -1)
   {
      printf("SEND OFFSET ERROR:%s\n", strerror(errno));
      return NULL;
   }
   printf("\nFILE:%s PART:%d\nNumber of bytes sent:%ld\n", fa->file, fa->n, sendAddr);
   close(fd);
   return (void *)sendAddr;
}
