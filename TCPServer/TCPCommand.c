#include "TCPCommand.h"

void listFiles(int socket)
{
   DIR *d;
   struct dirent *dir;
   d = opendir(".");
   int i = 0,
       imax = 1024;
   char *fileName = (char *)calloc(imax, sizeof(char));
   fileName[0] = '\0';

   if (d)
   {
      while ((dir = readdir(d)) != NULL)
      {
         if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
         {
            if (strlen(dir->d_name) + strlen(fileName) >= imax)
            {
               imax *= 2;
               fileName = (char *)realloc(fileName, imax * sizeof(char));
            }

            strcat(fileName, dir->d_name);
            strcat(fileName, "\n");
         }
      }
      sendString(fileName, socket);
      free(fileName);
      closedir(d);
   }
   return;
}