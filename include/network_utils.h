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

// TODO: change these ports accordingly to server/sm if needed
#define SM_PORT 8080
#define SERVER_PORT 8000    // temporary, sm does not send back ip and port currently
#define BASE 10

struct socket_network
{
    // cppcheck-suppress unusedStructMember
    char *address;
    // cppcheck-suppress unusedStructMember
    int sockfd;
    // cppcheck-suppress unusedStructMember
    struct sockaddr_storage addr;
    // cppcheck-suppress unusedStructMember
    socklen_t addr_len;
    // cppcheck-suppress unusedStructMember
    char *port;
};

void handle_arguments(int argc, char *argv[], struct socket_network *net_socket, int *err);

void socket_create(struct socket_network *net_socket, int *err);

// void socket_set_non_blocking(struct socket_network *net_socket, int *err);

void setup_network_address(struct socket_network *net_socket, int *err);

void socket_connect(int sockfd, const struct sockaddr *addr, socklen_t addr_len, int *err);

void socket_close(int sockfd);

void setup_socket(struct socket_network *net_socket, int *err);

#endif    // NETWORK_UTILS_H
