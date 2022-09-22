#ifndef TCPCOMMAND_H
#define TCPCOMMAND_H

#include "SendRecv.h"
#include <dirent.h>

void TCPCommandClient(int socket, int fd, char *argv[], int argc, int n_part, int n_file); // ok
int TCPCommandErrorFinder(int argc, char *argv[]);                                         // ok

#endif