#include "TCPCommand.h"
#include <sys/mman.h>

void TCPCommandClient(int socket, int fd, char *argv[], int argc, int n_part, int n_file)
{
   if (strcmp(argv[3], "list") == 0)
   {
      sendInt(COMMAND_LIST, socket);      // send the command to the server
      printf("%s\n", recvString(socket)); // receive the response
   }
   else if (strcmp(argv[3], "stat") == 0)
   {
      sendInt(COMMAND_STAT, socket);
      int timeSpent = (int)recvDouble(socket);
      int fileCount = recvInt(socket),
          fileSent = recvInt(socket),
          clientConnect = recvInt(socket);
      long int bytesSent = recvLInt(socket);
      int horas = timeSpent / 3600, minutos = (timeSpent % 3600) / 60, segundos = timeSpent % 60;
      printf("upTime:%d horas, %d minutos e %d segundos\nfileCount:%d\nfileSent:%d\nbytesSent:%ld\nclientConnect:%d\n", horas, minutos, segundos, fileCount, fileSent, bytesSent, clientConnect);
   }
   else
   {
      int i, j = 1;
      sendInt(COMMAND_DOWNLOAD, socket); // send the command to the server
      sendInt(n_part, socket);           // send the part the client want to download
      sendInt(atoi(argv[5]), socket);    // send how many parts the client want to receive the file
      sendString(argv[5 + n_file], socket);
      if (recvInt(socket) == 0)
      {
         printf("Downloading the part %d/%d of the file %s\n", n_part, atoi(argv[5]), argv[5 + n_file]);
         recvFile(socket, fd, n_part);
         printf("Downloading the part %d/%d of the file %s is complete!\n", n_part, atoi(argv[5]), argv[5 + n_file]);
      }
      else
      {
         printf("It's not possible to download the file %s\n", argv[5 + n_file]);
      }
   }

   return;
}

int TCPCommandErrorFinder(int argc, char *argv[])
{
   if (argc < 4)
   {
      printf("USAGE:\n");
      printf(" (1)list the archives of the server: ./client serverIpAddress port_number list\n");
      printf(" (2)download the file fd in n parts from the server: ./client serverIpAddress port_number download directory n_parts_total files\n");
      printf(" (3)show the statistic from the server: ./client serverIpAddress port_number stat\n");

      return EXIT_FAILURE;
   }

   if (strcmp(argv[3], "list") != 0)
   {
      if (strcmp(argv[3], "download") != 0)
      {
         if (strcmp(argv[3], "stat") != 0)
         {
            printf("COMMAND ERROR: Command %s not defined\n", argv[3]);
            return EXIT_FAILURE;
         }
      }
   }
   if (strcmp(argv[3], "download") == 0)
   {
      if (!chdir(argv[4]))
         printf("Change directory is complete!\n");
      else
      {
         printf("Change directory was a failure!\n");
         return EXIT_FAILURE;
      }

      if (atoi(argv[5]) <= 0)
      {
         printf("COMMAND ERROR: The fourth argument must be an integer greater than 0\n");
         return EXIT_FAILURE;
      }
   }

   return EXIT_SUCCESS;
}