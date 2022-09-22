#include "SendRecv.h"
#include "TCPCommand.h"
#include <pthread.h>

typedef struct clientResquest
{
   int socket;
   char **argv;
   int argc;
   int n_part;
   int n_file;
   int fd;
} clientResquest;

void *connectSocket(void *client);

int main(int argc, char *argv[])
{
   if (TCPCommandErrorFinder(argc, argv))
      return EXIT_FAILURE;

   char *serverIpAddress, *port_number;

   serverIpAddress = (char *)calloc(strlen(argv[1]) + 1, sizeof(char));
   port_number = (char *)calloc(strlen(argv[2]) + 1, sizeof(char));
   strcpy(serverIpAddress, argv[1]);
   strcpy(port_number, argv[2]);

   int len;
   struct sockaddr_in dest;

   memset(&dest, 0, sizeof(dest)); /* zero the struct */
   dest.sin_family = AF_INET;
   dest.sin_addr.s_addr = inet_addr(serverIpAddress); /* set destination IP number - localhost, 127.0.0.1*/
   dest.sin_port = htons(atoi(port_number));          /* set destination port number */

   if (strcmp(argv[3], "download") == 0)
   {

      int n_threads, n_file,
          i = 0, j = 0, n = 0, k = 0;

      n_threads = atoi(argv[5]);
      n_file = argc - 6;

      int *mysocket = (int *)calloc(n_threads * n_file, sizeof(int));

      for (; i < n_threads * n_file; i++)
      {
         mysocket[i] = socket(AF_INET, SOCK_STREAM, 0);
         if (mysocket[i] == -1)
         {
            printf("SOCKET ERROR: %s\n", strerror(errno));
            return EXIT_FAILURE;
         }
      }

      clientResquest *client;

      pthread_t *id = (pthread_t *)calloc(n_file * n_threads, sizeof(pthread_t));

      int *fd = (int *)calloc(n_file, sizeof(int));
      int f_open_error = 0;
      for (i = 0; i < n_file; i++)
      {
         fd[i] = open(argv[6 + i], O_CREAT | O_RDWR, S_IRWXU);
         if (fd[i] == -1)
         {
            printf("FILE OPEN ERROR:%s\n", strerror(errno));
            f_open_error++;
         }
         else
         {
            for (j = 0; j < n_threads; j++, n++)
            {
               client = (clientResquest *)malloc(sizeof(clientResquest));
               client->socket = mysocket[n];
               client->n_file = i + 1;
               client->n_part = j + 1;
               client->argc = argc;
               client->argv = (char **)calloc(argc, sizeof(char *));
               client->fd = fd[i];
               for (k = 0; k < argc; k++)
               {
                  client->argv[k] = (char *)calloc(strlen(argv[k]), sizeof(char));
                  strcpy(client->argv[k], argv[k]);
               }

               /* CONNECT */
               int connectResult = connect(client->socket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));

               if (connectResult == -1)
               {
                  printf("CLIENT ERROR: %s\n", strerror(errno));
                  return EXIT_FAILURE;
               }

               /* ------------------------------------- */

               /* THREAD CREATE */
               int erroThread = pthread_create(&id[n], NULL, connectSocket, client);

               if (erroThread != 0)
               {
                  printf("THREAD ERROR: %s\n", strerror(errno));
                  return EXIT_FAILURE;
               }

               /* ------------------------------------- */
            }
         }
      }

      for (i = 0; i < n_file * n_threads; i++)
         pthread_join(id[i], NULL);
      for (i = 0; i < n_file; i++)
      {
         close(fd[i]);
         close(mysocket[i]);
      }
      int finishSocket = socket(AF_INET, SOCK_STREAM, 0);
      if (finishSocket == -1)
      {
         printf("SOCKET ERROR: %s\n", strerror(errno));
         return EXIT_FAILURE;
      }
      int connectResult = connect(finishSocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
      sendInt(COMMAND_FINISH, finishSocket);
      sendInt(n_file - f_open_error, finishSocket);
   }
   else
   {
      int mysocket;
      mysocket = socket(AF_INET, SOCK_STREAM, 0);
      int connectResult = connect(mysocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
      if (connectResult == -1)
      {
         printf("CLIENT ERROR: %s\n", strerror(errno));
         return EXIT_FAILURE;
      }
      TCPCommandClient(mysocket, 0, argv, argc, 0, 0);
      close(mysocket);

      int finishSocket = socket(AF_INET, SOCK_STREAM, 0);
      if (finishSocket == -1)
      {
         printf("SOCKET ERROR: %s\n", strerror(errno));
         return EXIT_FAILURE;
      }
      connectResult = connect(finishSocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));
      sendInt(COMMAND_FINISH, finishSocket);
      sendInt(0, finishSocket);
   }

   printf("End of transmission with the server!\n");

   return EXIT_SUCCESS;
}

void *connectSocket(void *client)
{
   clientResquest *clientInfo = (clientResquest *)client;
   TCPCommandClient(clientInfo->socket, clientInfo->fd, clientInfo->argv, clientInfo->argc, clientInfo->n_part, clientInfo->n_file);

   close(clientInfo->socket);

   free(clientInfo);
   return NULL;
}