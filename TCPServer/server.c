#include "SendRecv.h"
#include "TCPCommand.h"
#include <pthread.h>
#include <sys/stat.h>
#include <sys/mman.h>

long int bytesSent = 0;

int fileSent = 0,
    clientConnect = 0;
double file_download_Error = 0;

pthread_mutex_t mutex_file_download_Error;
pthread_mutex_t mutex_filesSent;
pthread_mutex_t mutex_bytesSent;

struct timeval tv1;

typedef struct clientData
{
   int consocket;
   int mysocket;
   struct sockaddr_in *dest;
   socklen_t *socksize;
   pthread_t threadClient;
} clientData;

void *handleClient(void *consocket);
int filesCountDisc();
int acceptClient(void *Client);

int main(int argc, char *argv[])
{
   pthread_mutex_init(&mutex_bytesSent, NULL);
   pthread_mutex_init(&mutex_filesSent, NULL);
   pthread_mutex_init(&mutex_file_download_Error, NULL);
   gettimeofday(&tv1, NULL);
   /* COMMAND LINE FAILURE */
   if (argc < 2 || argc > 3)
   {
      printf("USAGE: ./server port_number directory(optional)\n");
      return EXIT_FAILURE;
   }
   /* ---------------------------------------------------- */
   /* CHANGE DIRECTORY */
   if (argc == 3)
   {
      if (!chdir(argv[2]))
         printf("Change directory is complete!\n");
      else
         printf("Change directory was a failure. Using the default directory!\n");
   }
   /* ---------------------------------------------------- */
   srand(time(NULL));
   /* CONNECTION */
   struct sockaddr_in dest; /* socket info about the machine connecting to us */
   struct sockaddr_in serv; /* socket info about our server */
   int mysocket;            /* socket used to listen for incoming connections */
   socklen_t socksize = sizeof(struct sockaddr_in);

   memset(&serv, 0, sizeof(serv));           /* zero the struct before filling the fields */
   serv.sin_family = AF_INET;                /* set the type of connection to TCP/IP */
   serv.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
   serv.sin_port = htons(atoi(argv[1]));     /* set the server port number */

   mysocket = socket(AF_INET, SOCK_STREAM, 0);

   /* bind serv information to mysocket */
   bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));

   /* start listening, allowing a queue of up to 1024 pending connection */
   listen(mysocket, 1024);

   printf("Server is waiting for connections on port:%s\n", argv[1]);

   clientData *Client = (clientData *)malloc(sizeof(clientData));

   Client->mysocket = mysocket;
   Client->dest = &dest;
   Client->socksize = &socksize;

   int consocket;
   consocket = acceptClient(Client);

   /* ---------------------------------------------------- */
   while (1)
   {
      Client = (clientData *)malloc(sizeof(clientData));
      Client->consocket = consocket;
      Client->mysocket = mysocket;
      Client->dest = &dest;
      Client->socksize = &socksize;

      int erroThread = pthread_create(&Client->threadClient, NULL, handleClient, Client);
      if (erroThread != 0)
      {
         printf("THREAD ERROR: %s\n", strerror(errno));
         return EXIT_FAILURE;
      }

      consocket = acceptClient(Client);
   }

   close(mysocket);
   pthread_mutex_destroy(&mutex_filesSent);
   pthread_mutex_destroy(&mutex_bytesSent);
   pthread_mutex_destroy(&mutex_file_download_Error);
   return EXIT_SUCCESS;
}

int acceptClient(void *Client)
{
   clientData *newClient = (clientData *)Client;
   int mysocket = newClient->mysocket;
   struct sockaddr_in *dest = newClient->dest;
   socklen_t *socksize = newClient->socksize;

   int consocket = accept(mysocket, (struct sockaddr *)dest, socksize);
   if (consocket == -1)
   {
      printf("ACCEPT ERROR: %s\n", strerror(errno));
      return -1;
   }
   printf("Incoming connection from %s\n", inet_ntoa(dest->sin_addr));
   return consocket;
}

void *handleClient(void *socket)
{

   clientData *newClient = (clientData *)socket;

   int consocket = newClient->consocket;

   fileInfo *file = (fileInfo *)malloc(sizeof(fileInfo));

   struct timeval tv2;

   double timeSpent;

   int n_parts = 1,
       n_Total,
       i = 0,
       nFile;

   long int temp = 0;

   switch (recvInt(consocket))
   {
   case COMMAND_LIST:
      listFiles(consocket);
      break;

   case COMMAND_STAT:
      gettimeofday(&tv2, NULL);
      timeSpent = (double)(tv2.tv_usec - tv1.tv_usec) / 1000000 + (double)(tv2.tv_sec - tv1.tv_sec);
      int horas = (int)timeSpent / 3600, minutos = ((int)timeSpent % 3600) / 60, segundos = (int)timeSpent % 60;
      printf("%lf\n", timeSpent);
      printf("upTime:%d horas, %d minutos e %d segundos\n", horas, minutos, segundos);
      sendDouble(timeSpent, consocket);
      sendInt(filesCountDisc(), consocket);
      sendInt(fileSent, consocket);
      sendInt(clientConnect, consocket);
      sendLInt(bytesSent, consocket);
      break;

   case COMMAND_DOWNLOAD:

      n_parts = recvInt(consocket);
      n_Total = recvInt(consocket);
      file->file = (char *)calloc(1025, sizeof(char));
      strcpy(file->file, recvString(consocket));
      file->nTotal = n_Total;
      file->socket = consocket;
      file->n = n_parts;

      int fd = open(file->file, O_RDONLY);
      if (fd == -1)
      {
         printf("FILE %s OPEN ERROR:%s\n", file->file, strerror(errno));
         sendInt(-1, consocket);
         pthread_mutex_lock(&mutex_file_download_Error);
         file_download_Error += 1 / n_Total;
         pthread_mutex_unlock(&mutex_file_download_Error);
         return NULL;
      }
      sendInt(0, consocket);

      struct stat sb;
      if (stat(file->file, &sb) == -1)
      {
         perror("stat");
         exit(EXIT_FAILURE);
      }

      char *addr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
      if (addr == MAP_FAILED)
      {
         printf("MMAP ERROR:%s\n", strerror(errno));
         return NULL;
      }
      long int temp = (long int)sendFile(file, fd, addr);
      pthread_mutex_lock(&mutex_bytesSent);
      bytesSent += temp;
      pthread_mutex_unlock(&mutex_bytesSent);

      free(file->file);
      break;
   case COMMAND_FINISH:
      nFile = recvInt(consocket);
      pthread_mutex_lock(&mutex_filesSent);
      fileSent += nFile - file_download_Error;
      clientConnect++;
      pthread_mutex_unlock(&mutex_filesSent);
   }

   close(consocket);
   free(socket);
   return NULL;
}

int filesCountDisc()
{
   int fileCount = -2;
   DIR *d;
   struct dirent *dir;
   d = opendir(".");
   if (d)
   {
      while ((dir = readdir(d)) != NULL)
      {
         fileCount++;
      }
      closedir(d);
   }
   return fileCount;
}