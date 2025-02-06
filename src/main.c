#include "messages.h"
#include "network_utils.h"
#include "packet.h"
#include "signup_form.h"
#include "start_menu.h"
#include <ncurses.h>

int main(int argc, char *argv[])
{
    struct socket_network net_socket;
    // struct ConnectionMessage connection_message;
    struct Message          message;
    struct ACC_Create_Login acc_create;

    int  res;
    bool success = false;
    int  err     = 0;

    // connection_message.active_server_ip = NULL;

    acc_create.message  = &message;
    acc_create.seq_len  = NULL;
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
    socket_connect(net_socket.sockfd, (struct sockaddr *)(&(net_socket.addr)), net_socket.addr_len, &err);
    if(err != 0)
    {
        goto cleanup;
    }
    // end socket connect

    res = display_menu();

    while(!success)
    {
        uint8_t type;
        uint8_t version;
        uint8_t id;
        uint8_t payload_len;

        // one form now, for login and create account
        start_signup_form(&acc_create, res, &err);
        if(err != 0)
        {
            goto cleanup;
        }

        // if res == 1. send login request

        // if(res == 1)
        // {
        //     type = LOGIN_REQUEST;
        // }
        // else if(res == 2)
        // {
        //     type = ACCOUNT_CREATE;
        // }
        // if res == 2. send create account request
        type        = LOGIN_REQUEST;
        version     = 0x01;
        id          = 0x01;
        payload_len = (uint8_t)(strlen((char *)acc_create.username) + strlen((char *)acc_create.username));

        construct_message(&message, type, version, id, payload_len);

        send_and_serialize_ACC_Create_Login(net_socket.sockfd, &acc_create);
    }

    printf("client ran successfully\n");
cleanup:
    socket_close(net_socket.sockfd);
    free_acc_create(&acc_create);
done:
    return 0;
}
