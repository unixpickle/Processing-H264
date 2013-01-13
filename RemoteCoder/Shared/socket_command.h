//
//  socket_command.h
//  RemoteCoder
//
//  Created by Alex Nichol on 1/13/13.
//
//

#ifndef RemoteCoder_socket_command_h
#define RemoteCoder_socket_command_h

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/un.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

int main_listen_client(int argc, const char ** argv);
int listen_method(int method, const char * source, int allowRemote);
int accept_method(int method, int server);

#endif
