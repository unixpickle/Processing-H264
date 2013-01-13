//
//  socket_command.c
//  RemoteCoder
//
//  Created by Alex Nichol on 1/13/13.
//
//

#include "socket_command.h"
#include <signal.h>
#include <errno.h>

static void sigchild_handler(int sig);

int main_listen_client(int argc, const char ** argv) {
    if (argc < 3) {
        fprintf(stderr, "Usgae: %s <unix | socket> <file | port>\n", argv[0]);
        return -1;
    }
    
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = &sigchild_handler;
    sigaction(SIGCHLD, &sa, NULL);
    
    int medium = 0; // 0 = network, 1 == unix
    if (strcmp(argv[1], "unix") == 0) {
        medium = 1;
    } else if (strcmp(argv[1], "socket") == 0) {
        medium = 0;
    } else {
        fprintf(stderr, "error: unknown communication type %s\n", argv[1]);
        return -1;
    }
    
    int serverSocket = listen_method(medium, argv[2], 1);
    if (serverSocket < 0) {
        return -2;
    }
    
    // We will only allow one connection at once. This means that the parent
    // process should connect immediately.
    
    while (1) {
        int client = accept_method(medium, serverSocket);
        if (client < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            return -2;
        }
        int i = fork();
        if (i == 0) {
            close(serverSocket);
            return client;
        }
    }
}

int listen_method(int method, const char * source, int allowRemote) {
    int server;
    if (method == 0) {
        // inet socket
        int port = atoi(source);
        struct sockaddr_in local;
        if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("socket");
            return -1;
        }
        int on = 1;
        setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = INADDR_ANY; // TODO: allowLocal
        local.sin_port = htons(port);
        if (bind(server, (struct sockaddr *)&local, sizeof(local)) < 0) {
            perror("bind");
            return -1;
        }
    } else {
        // unix socket
        struct sockaddr_un local;
        if (strlen(source) + 1 > sizeof(local.sun_path)) {
            fprintf(stderr, "listen_method: path too long");
            return -1;
        }
        if ((server = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
            perror("socket");
            return -1;
        }
        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, source);
        if (unlink(source) != 0) {
            perror("unlink");
            return -1;
        }
        int length = (int)strlen(source) + (int)sizeof(local.sun_family);
        if (bind(server, (struct sockaddr *)&local, length) < 0) {
            perror("bind");
            return -1;
        }
    }
    if (listen(server, 5) < 0) {
        perror("listen");
        return -1;
    }
    return server;
}

int accept_method(int method, int server) {
    if (method == 0) {
        // inet socket
        struct sockaddr_in remote;
        socklen_t x = sizeof(remote);
        return accept(server, (struct sockaddr *)&remote, &x);
    } else {
        // unix socket
        struct sockaddr_un remote;
        socklen_t x = sizeof(remote);
        return accept(server, (struct sockaddr *)&remote, &x);
    }
}

static void sigchild_handler(int sig) {
    int status;
    wait(&status);
}
