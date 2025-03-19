#include "chat_screen.h"
#include "error_message.h"
#include "messages.h"
#include "packet.h"
#include "signals.h"
#include "signup_form.h"
#include "start_menu.h"
#include <ncurses.h>

#define PACKETLEN 777

int     login_or_create(struct Message request_header, int sockfd, int form_type, int *err);
uint8_t make_login_create_req(struct Message *header, struct ACC_Create_Login request, int form_type);
void    set_packet_type(int form_type, uint8_t *type);
bool    handle_login_res(struct Message incoming_message, const uint8_t *incoming_stream, uint8_t *username, int sockfd);
bool    handle_create_res(struct Message incoming_message, const uint8_t *incoming_stream);

// Client-ServerManager functions
void make_ip_req(int server_manager_fd, struct ConnectionMessage *connection_message, int *err);

// end Client-ServerManager functions
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
int terminate = 0;

// assign packet type depending on login or create
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

// handle login response packet
bool handle_login_res(struct Message incoming_message, const uint8_t *incoming_stream, uint8_t *username, int sockfd)
{
    if(incoming_message.packet_type == SYS_Error)
    {
        uint8_t  error_message[PACKETLEN];    // buffer to hold the error message send from the server
        uint8_t *error_code;                  // error code of response
        parse_and_extract_message(incoming_stream, error_message, ERROR_MESSSAGE_INDEX, (size_t)incoming_message.payload_len - ERROR_CODE_ENCODED);

        error_code = get_error_code(incoming_stream, 1);

        display_error_message(error_message, error_code, (size_t)incoming_message.payload_len - 3, 1);
        free(error_code);
    }
    else if(incoming_message.packet_type == LOGIN_SUCCESS)
    {
        get_user_id(incoming_stream, &incoming_message.sender_id);
        printf("user idddd: %d\n", incoming_message.sender_id);
        start_chat_screen(incoming_message.sender_id, username, sockfd);
        make_logout_req(sockfd, incoming_message.sender_id);
        return true;
    }

    return false;
}

// handle create account response packet
bool handle_create_res(struct Message incoming_message, const uint8_t *incoming_stream)
{
    if(incoming_message.packet_type == SYS_Error)
    {
        uint8_t  error_message[PACKETLEN];    // buffer to hold the error message send from the server
        uint8_t *error_code;                  // error code of response
        parse_and_extract_message(incoming_stream, error_message, ERROR_MESSSAGE_INDEX, (size_t)incoming_message.payload_len - ERROR_CODE_ENCODED);

        error_code = get_error_code(incoming_stream, 1);

        display_error_message(error_message, error_code, (size_t)incoming_message.payload_len - ERROR_CODE_ENCODED, 1);

        free(error_code);
    }
    else if(incoming_message.packet_type == SYS_Success)
    {
        return true;
    }

    return false;
}

// prepare the request header for login and create request
uint8_t make_login_create_req(struct Message *header, struct ACC_Create_Login request, int form_type)
{
    uint8_t  type;
    uint8_t  version;
    uint16_t id;
    uint16_t payload_len;
    size_t   username_encoded_len;
    size_t   password_encoded_len;

    version = VERSION;
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

// loop for handling login or create account requests
int login_or_create(struct Message request_header, int sockfd, int form_type, int *err)
{
    struct ACC_Create_Login acc_create_login;              // struct to form account create or login request
    uint8_t                 incoming_stream[PACKETLEN];    // buffer to hold the request read from the server
    // size_t                  input_size;                    // size of the request packet that was read
    struct Message incoming_message;    // struct to store the response header info
    bool           success    = false;
    acc_create_login.message  = &request_header;
    acc_create_login.username = NULL;
    acc_create_login.password = NULL;

    while(!success && !terminate)
    {
        ssize_t bytes_read;
        uint8_t type;

        // one form now, for login and create account
        start_signup_form(&acc_create_login, form_type, err);
        if(*err != 0)
        {
            free_acc_create(&acc_create_login);
            perror("opening form");
            return -1;
        }
        if(terminate)
        {
            break;
        }

        type = make_login_create_req(&request_header, acc_create_login, form_type);

        // send packet that either represents account_create or login
        send_and_serialize_ACC_Create_Login(sockfd, &acc_create_login);

        // read the response into buffer
        // read_entire_stream(sockfd, &incoming_stream, &input_size, err);
        bytes_read = read(sockfd, incoming_stream, PACKETLEN);
        if(bytes_read < 0)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        incoming_stream[bytes_read] = '\0';

        parse_response_header(incoming_stream, &incoming_message);

        if(type == LOGIN_REQUEST)
        {
            success = handle_login_res(incoming_message, incoming_stream, acc_create_login.username, sockfd);
        }
        else if(type == ACCOUNT_CREATE)
        {
            success = handle_create_res(incoming_message, incoming_stream);
        }

        free_acc_create(&acc_create_login);
    }

    return *err;
}

void make_ip_req(const int server_manager_fd, struct ConnectionMessage *connection_message, int *err)
{
    size_t   stream_size;
    uint8_t  message_type;
    uint8_t  version;
    uint8_t *incoming_stream;
    // connection_message->active_server_ip = [PACKETLEN];

    message_type = 0x00;
    version      = VERSION;

    construct_connection_message(connection_message, message_type, version, 0x00);

    send_and_serialize_connection_message(server_manager_fd, connection_message);

    read_entire_stream(server_manager_fd, &incoming_stream, &stream_size, err);
    if(*err != 0)
    {
        free(incoming_stream);
        return;
    }

    if(stream_size == 0)
    {
        return;
    }

    printf("Retrieved packet of size %zu\n", stream_size);
    send_packet_t(incoming_stream, stream_size);

    parse_connection_message_header(incoming_stream, connection_message);
    if(connection_message->server_online != 0)
    {
        // TODO: uncomment port lines when sm implements port
        const size_t ip_index = 5;
        size_t       len;
        size_t       port_index;
        len = (size_t)incoming_stream[ip_index - 1];    // retrieves the server ip length
        parse_and_extract_message(incoming_stream, connection_message->active_server_ip, ip_index, len);
        // connection_message->active_server_ip = active_server_ip;
        // prints server ip
        printf("\nserver ip length: %zu\n", len);
        send_packet_t(connection_message->active_server_ip, len);

        port_index = ip_index + len + 1;    // retrieves the server port index
        len        = (size_t)incoming_stream[port_index - 1];
        parse_and_extract_message(incoming_stream, connection_message->active_server_port, port_index, len);
        // connection_message->active_server_port = active_server_port;
        // prints server ip
        printf("\nserver port length: %zu\n", len);
        send_packet_t(connection_message->active_server_port, len);
    }
    free(incoming_stream);
}

/*
    -h server manager ip
*/
int main(int argc, char *argv[])
{
    struct socket_network net_socket;    // network socket info
    int                   res;
    struct Message        request_header = {0};    // struct to form request header
    // struct ConnectionMessage connection_message = {0};
    int err = 0;
    // net_socket.port                             = SM_PORT;
    net_socket.port = SERVER_PORT;

    // connection_message.active_server_ip = NULL;

    handle_arguments(argc, argv, &net_socket, &err);
    if(err != 0)
    {
        goto done;
    }

    // socket initialization (sm)
    // setup_socket(&net_socket, &err);
    // if(err != 0)
    // {
    //     goto cleanup;
    // }

    // // client-sm communication
    // net_socket.address = NULL;    // set to null, unneeded

    // // send active server ip request
    // make_ip_req(net_socket.sockfd, &connection_message, &err);
    // if(err != 0)
    // {
    //     goto cleanup;
    // }
    // // TODO: uncomment this when sm is done
    // // if(connection_message.server_online == 0)
    // // {
    // //     printf("No active server.\n");
    // //     goto cleanup;
    // // }

    // close(net_socket.sockfd);
    // end connection with sm

    // TODO: must connect to server ip and port provided from sm
    // net_socket.address = (char *)connection_message.active_server_ip; // uncomment this line
    // net_socket.port = (char *)connection_message.port;

    // net_socket.address = strdup("127.0.0.2");    // TODO: change this to appropriate server ip
    // net_socket.port    = SERVER_PORT;

    // socket initialization (server)
    setup_socket(&net_socket, &err);
    if(err != 0)
    {
        goto cleanup;
    }

    setup_signal(sigint_handler, SIGINT, &err);
    if(err != 0)
    {
        goto cleanup;
    }

    res = display_menu();
    if(terminate)
    {
        goto cleanup;
    }

    res = login_or_create(request_header, net_socket.sockfd, res, &err);
    if(res == -1)
    {
        printf("error opening form\n");
        goto cleanup;
    }
    if(terminate)
    {    // SIGINT received in login or account creation page
        goto cleanup;
    }

    printf("client ran successfully\n");
cleanup:

    // free(connection_message.active_server_ip);
    // if(net_socket.address != NULL)
    // {
    //     free(net_socket.address);
    //     net_socket.address = NULL;
    // }
    close(net_socket.sockfd);
done:
    exit(EXIT_SUCCESS);
}
