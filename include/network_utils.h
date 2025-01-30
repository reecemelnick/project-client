#ifndef NETWORK_UTILS_H
#define NETWORK_UTILS_H

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080

struct socket_network 
{
    char                   *address;
    int                     sockfd;
    struct sockaddr_storage addr;
    socklen_t               addr_len;
};

void setup_signal(void (*handler)(int), int *err);

void handle_arguments(int argc, char *argv[], struct socket_network *net_socket, int *err);

void socket_create(struct socket_network *net_socket, int *err);

// void socket_set_non_blocking(struct socket_network *net_socket, int *err);

void setup_network_address(struct socket_network *net_socket, int *err);

void socket_connect(int sockfd, const struct sockaddr *addr, socklen_t addr_len, int *err);

void socket_close(int sockfd);

#endif    // NETWORK_UTILS_H
