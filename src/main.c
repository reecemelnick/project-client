#include "chat_screen.h"
#include "error_message.h"
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
    uint8_t                *incoming_stream;    // NOLINT
    size_t                  input_size;
    struct Message          incoming_message;
    uint8_t                *error_message;
    uint8_t                *error_code;
    uint16_t               *user_id;

    int  res;
    bool success = false;
    int  err     = 0;

    // connection_message.active_server_ip = NULL;

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
    socket_connect(net_socket.sockfd, (struct sockaddr *)(&(net_socket.addr)), net_socket.addr_len, &err);
    if(err != 0)
    {
        goto cleanup;
    }
    // end socket connect

    res = display_menu();

    while(!success)
    {
        uint8_t  type;
        uint8_t  version;
        uint16_t id;
        uint16_t payload_len;

        // one form now, for login and create account
        start_signup_form(&acc_create, res, &err);
        if(err != 0)
        {
            goto cleanup;
        }

        version = 0x01;
        id      = 0x01;
        if(acc_create.username != NULL && acc_create.password != NULL)
        {
            payload_len = (uint16_t)(strlen((const char *)acc_create.username) + strlen((const char *)acc_create.password));
        }
        else
        {
            break;
        }

        if(res == 1)
        {
            type = LOGIN_REQUEST;
        }
        else if(res == 2)
        {
            type = ACCOUNT_CREATE;
        }
        else
        {
            type = 0;
        }

        // construct a message that will be sent to server
        construct_message(&message, type, version, id, payload_len);

        // send packet that either represents account_create or login
        send_and_serialize_ACC_Create_Login(net_socket.sockfd, &acc_create);

        // read the response into buffer
        incoming_stream = read_entire_stream(net_socket.sockfd, &input_size, &err);

        send_packet_t(incoming_stream, input_size);    // print (TEMP)

        parse_response_header(incoming_stream, &incoming_message);

        if(type == LOGIN_REQUEST)
        {
            if(incoming_message.packet_type == SYS_Error)
            {
                error_message = parse_and_extract_message(incoming_stream, 11, (size_t)incoming_message.payload_len - 3, &err);    // NOLINT

                error_code = get_error_code(incoming_stream, 1);

                display_error_message(error_message, error_code, (size_t)incoming_message.payload_len - 3, 1);

                free(error_message);
                free(error_code);
            }
            else if(incoming_message.packet_type == LOGIN_SUCCESS)
            {
                user_id = get_user_id(incoming_stream);
                start_chat_screen(user_id);
                success = true;
                free(user_id);
            }
        }
        else if(type == ACCOUNT_CREATE)
        {
            if(incoming_message.packet_type == SYS_Error)
            {
                error_message = parse_and_extract_message(incoming_stream, 11, (size_t)incoming_message.payload_len - 3, &err);    // NOLINT

                error_code = get_error_code(incoming_stream, 1);

                display_error_message(error_message, error_code, (size_t)incoming_message.payload_len - 3, 1);

                free(error_message);
                free(error_code);
            }
            else if(incoming_message.packet_type == SYS_Success)
            {
                success = true;
            }
        }

        free_acc_create(&acc_create);
        free(incoming_stream);
    }

    printf("client ran successfully\n");
cleanup:
    socket_close(net_socket.sockfd);
done:
    return 0;
}
