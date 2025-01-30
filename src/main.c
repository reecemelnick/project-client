
#include "login_form.h"
#include "network_utils.h"
#include "signup_form.h"
#include "start_menu.h"
#include <ncurses.h>

int main(int argc, char *argv[])
{
    struct socket_network net_socket;
    struct Message        message;
    struct ACC_Create     acc_create;

    int res;
    int err = 0;

    acc_create.message  = &message;
    acc_create.username = NULL;
    acc_create.password = NULL;

    handle_arguments(argc, argv, &net_socket, &err);
    if(err != 0)
    {
        goto done;
    }

    // socket initialization
    socket_create(&net_socket, &err);
    if(err != 0)
    {
        goto done;
    }

    setup_network_address(&net_socket, &err);
    if(err != 0)
    {
        goto cleanup;
    }
    // end socket initialization

    // socket connect
    // socket_connect(net_socket.sockfd, (struct sockaddr *)(&(net_socket.addr)), net_socket.addr_len, &err);
    // if(err != 0)
    // {
    //     goto cleanup;
    // }
    // end socket connect

    res = display_menu();
    if(res == 1)
    {
        start_login_form();
    }
    else if(res == 2)
    {
        start_signup_form(&acc_create, &err);
        if(err != 0)
        {
            goto cleanup;
        }
    }

    printf("client ran successfully");
cleanup:
    socket_close(net_socket.sockfd);
    free(acc_create.username);
    free(acc_create.password);
done:
    return 0;
}
