#include "../include/network_utils.h"

void setup_signal(void (*handler)(int), int *err)
{
    if(signal(SIGINT, handler) == SIG_ERR)
    {
        perror("Error setting up signal handler");
        *err = errno;
    }
}

void handle_arguments(int argc, char *argv[], struct socket_network *net_socket, int *err)
{
    int option;
    net_socket->address = NULL;
    while((option = getopt(argc, argv, "h:")) != -1)
    {
        if(option == 'h')
        {
            net_socket->address = optarg;
        }
        else
        {
            perror("Error invalid command line args");
            *err = 1;
        }
    }
    if(net_socket->address == NULL)
    {
        perror("Error unable to parse ip");
        *err = 1;
    }
}

void socket_create(struct socket_network *net_socket, int *err)
{
    net_socket->sockfd = socket(AF_INET, SOCK_STREAM, 0);    // NOLINT
    if(net_socket->sockfd == -1)
    {
        *err = errno;
        perror("Error creating socket... socket()");
    }
}

// void socket_set_non_blocking(struct socket_network *net_socket, int *err)
// {
//     // Returns flags of socket
//     int flags = fcntl(net_socket->sockfd, F_GETFL, 0);
//     if(flags == -1)
//     {
//         *err = errno;
//     }

//     // Sets non-blocking flag to socket
//     if(fcntl(net_socket->sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
//     {
//         *err = errno;
//     }
// }

void setup_network_address(struct socket_network *net_socket, int *err)
{
    memset(&(net_socket->addr), 0, sizeof((net_socket->addr)));

    if(inet_pton(AF_INET, net_socket->address, &(((struct sockaddr_in *)(&(net_socket->addr)))->sin_addr)) == 1)
    {
        struct sockaddr_in *ipv4_addr;

        net_socket->addr.ss_family = AF_INET;
        ipv4_addr                  = (struct sockaddr_in *)(&(net_socket->addr));
        ipv4_addr->sin_port        = htons(net_socket->port);
        net_socket->addr_len       = sizeof(*ipv4_addr);
    }
    else if(inet_pton(AF_INET6, net_socket->address, &(((struct sockaddr_in6 *)(&(net_socket->addr)))->sin6_addr)) == 1)
    {
        struct sockaddr_in6 *ipv6_addr;

        net_socket->addr.ss_family = AF_INET6;
        ipv6_addr                  = (struct sockaddr_in6 *)(&(net_socket->addr));
        ipv6_addr->sin6_port       = htons(net_socket->port);
        net_socket->addr_len       = sizeof(*ipv6_addr);
    }
    else
    {
        perror("Error address is neither ipv4 or ipv6");
        *err = errno;
    }
}

void socket_connect(int sockfd, const struct sockaddr *addr, socklen_t addr_len, int *err)
{
    if(connect(sockfd, addr, addr_len) != 0)
    {
        perror("Error while connecting socket");
        *err = errno;
    }
}

void socket_close(int sockfd)
{
    if(close(sockfd) != 0)
    {
        perror("Error closing socket");
    }
}

void setup_socket(struct socket_network *net_socket, int *err)
{
    socket_create(net_socket, err);
    if(*err != 0)
    {
        return;
    }

    setup_network_address(net_socket, err);
    if(*err != 0)
    {
        return;
    }
    // end socket initialization

    // socket connect
    socket_connect(net_socket->sockfd, (struct sockaddr *)(&(net_socket->addr)), net_socket->addr_len, err);
    if(*err != 0)
    {
        return;
    }
}
