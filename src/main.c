#include "chat_screen.h"
#include "error_message.h"
#include "messages.h"
#include "network_utils.h"
#include "packet.h"
#include "signup_form.h"
#include "start_menu.h"
#include <ncurses.h>

int     login_or_create(struct Message request_header, int sockfd, int form_type, int *err);
uint8_t make_login_create_req(struct Message *header, struct ACC_Create_Login request, int form_type);
void    set_packet_type(int form_type, uint8_t *type);
bool    handle_login_res(struct Message incoming_message, const uint8_t *incoming_stream, int *err);
bool    handle_create_res(struct Message incoming_message, const uint8_t *incoming_stream, int *err);

void set_packet_type(int form_type, uint8_t *type)
{
    if(form_type == 1)
    {
        *type = LOGIN_REQUEST;
    }
    else if(form_type == 2)
    {
        *type = ACCOUNT_CREATE;
    }
    else
    {
        *type = 0;
    }
}

bool handle_login_res(struct Message incoming_message, const uint8_t *incoming_stream, int *err)
{
    if(incoming_message.packet_type == SYS_Error)
    {
        uint8_t *error_message;    // buffer to hold the error message send from the server
        uint8_t *error_code;       // error code of response
        parse_and_extract_message(incoming_stream, &error_message, ERROR_MESSSAGE_INDEX, (size_t)incoming_message.payload_len - ERROR_CODE_ENCODED, err);

        error_code = get_error_code(incoming_stream, 1);

        display_error_message(error_message, error_code, (size_t)incoming_message.payload_len - 3, 1);

        free(error_message);
        free(error_code);
    }
    else if(incoming_message.packet_type == LOGIN_SUCCESS)
    {
        get_user_id(incoming_stream, &incoming_message.sender_id);
        start_chat_screen(incoming_message.sender_id);
        return true;
    }

    return false;
}

bool handle_create_res(struct Message incoming_message, const uint8_t *incoming_stream, int *err)
{
    if(incoming_message.packet_type == SYS_Error)
    {
        uint8_t *error_message;    // buffer to hold the error message send from the server
        uint8_t *error_code;       // error code of response
        parse_and_extract_message(incoming_stream, &error_message, ERROR_MESSSAGE_INDEX, (size_t)incoming_message.payload_len - ERROR_CODE_ENCODED, err);

        error_code = get_error_code(incoming_stream, 1);

        display_error_message(error_message, error_code, (size_t)incoming_message.payload_len - ERROR_CODE_ENCODED, 1);

        free(error_message);
        free(error_code);
    }
    else if(incoming_message.packet_type == SYS_Success)
    {
        return true;
    }

    return false;
}

uint8_t make_login_create_req(struct Message *header, struct ACC_Create_Login request, int form_type)
{
    uint8_t  type;
    uint8_t  version;
    uint16_t id;
    uint16_t payload_len;
    size_t   username_encoded_len;
    size_t   password_encoded_len;

    version = 0x01;
    id      = 0x00;

    username_encoded_len = strlen((const char *)request.username) + ENCODE_BYTES;
    password_encoded_len = strlen((const char *)request.password) + ENCODE_BYTES;
    payload_len          = (uint16_t)(username_encoded_len + password_encoded_len);
    printf("payload len: %u\n", payload_len);

    set_packet_type(form_type, &type);

    // construct a message that will be sent to server
    construct_message(header, type, version, id, payload_len);
    return type;
}

int login_or_create(struct Message request_header, int sockfd, int form_type, int *err)
{
    struct ACC_Create_Login acc_create_login;    // struct to form account create or login request
    uint8_t                *incoming_stream;     // buffer to hold the request read from the server
    size_t                  input_size;          // size of the request packet that was read
    struct Message          incoming_message;    // struct to store the response header info
    bool                    success = false;
    acc_create_login.message        = &request_header;
    acc_create_login.username       = NULL;
    acc_create_login.password       = NULL;

    while(!success)
    {
        uint8_t type;

        // one form now, for login and create account
        start_signup_form(&acc_create_login, form_type, err);
        if(*err != 0)
        {
            free_acc_create(&acc_create_login);
            perror("opening form");
            return -1;
        }

        type = make_login_create_req(&request_header, acc_create_login, form_type);

        // send packet that either represents account_create or login
        send_and_serialize_ACC_Create_Login(sockfd, &acc_create_login);

        // read the response into buffer
        read_entire_stream(sockfd, &incoming_stream, &input_size, err);

        send_packet_t(incoming_stream, input_size);    // print (TEMP)

        parse_response_header(incoming_stream, &incoming_message);

        if(type == LOGIN_REQUEST)
        {
            success = handle_login_res(incoming_message, incoming_stream, err);
        }
        else if(type == ACCOUNT_CREATE)
        {
            success = handle_create_res(incoming_message, incoming_stream, err);
        }

        free_acc_create(&acc_create_login);
        free(incoming_stream);
    }

    return *err;
}

int main(int argc, char *argv[])
{
    struct socket_network net_socket;              // network socket info
    struct Message        request_header = {0};    // struct to form request header
    int                   err            = 0;
    int                   res;

    // connection_message.active_server_ip = NULL;

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

    res = login_or_create(request_header, net_socket.sockfd, res, &err);
    if(res == -1)
    {
        printf("error opening form\n");
    }

    printf("client ran successfully\n");
cleanup:
    socket_close(net_socket.sockfd);
done:
    return 0;
}
