
#include "login_form.h"
#include "network_utils.h"
#include "signup_form.h"
#include "start_menu.h"
#include <ncurses.h>

int main(int argc, char *argv[])
{
    struct socket_network net_socket;
    // struct ConnectionMessage connection_message;
    struct Message          message;
    struct ACC_Create_Login acc_create_login;

    int res;
    int err = 0;

    // connection_message.active_server_ip = NULL;

    acc_create_login.message  = &message;
    acc_create_login.seq_len  = NULL;
    acc_create_login.username = NULL;
    acc_create_login.password = NULL;

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

    // We need to query server manager for active server ip first
    // construct_connection_message()
    // serialize_and_send_connection_message()

    res = display_menu();
    if(res == 1)
    {
        start_login_form();
    }
    else if(res == 2)
    {
        start_signup_form(&acc_create_login, &err);
        if(err != 0)
        {
            goto cleanup;
        }
    }

    printf("client ran successfully\n");
cleanup:
    socket_close(net_socket.sockfd);
    free_acc_create(&acc_create_login);
done:
    return 0;
}
