#include "login_signup.h"
#include "chat_screen.h"
#include "error_message.h"
#include "messages.h"
#include "packet.h"
#include "signals.h"
#include "signup_form.h"
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

static void set_packet_type(int form_type, uint8_t *type);
static int  handle_login_create_res(struct Message incoming_message, const uint8_t *incoming_stream, uint8_t *username, int sockfd);
static void make_login_create_req(struct Message *header, struct ACC_Create_Login request, int form_type);

// assign packet type depending on login or create
static void set_packet_type(int form_type, uint8_t *type)
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

// handle login and create account response packet
static int handle_login_create_res(struct Message incoming_message, const uint8_t *incoming_stream, uint8_t *username, int sockfd)
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

        // ENTER CHAT ROOM
        start_chat_screen(incoming_message.sender_id, username, sockfd);

        // send logout request
        make_logout_req(sockfd, incoming_message.sender_id);
        return 1;
    }
    else if(incoming_message.packet_type == SYS_Success)
    {
        return 1;
    }

    return 0;
}

// prepare the request header for login and create request
static void make_login_create_req(struct Message *header, struct ACC_Create_Login request, int form_type)
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
}

// loop for handling login or create account requests
int login_or_create(struct Message request_header, int sockfd, int form_type, int *err)
{
    struct ACC_Create_Login acc_create_login;              // struct to form account create or login request
    uint8_t                 incoming_stream[PACKETLEN];    // buffer to hold the request read from the server
    // size_t                  input_size;                    // size of the request packet that was read
    struct Message incoming_message;    // struct to store the response header info
    int            success    = 0;
    acc_create_login.message  = &request_header;
    acc_create_login.username = NULL;
    acc_create_login.password = NULL;

    // loop of trying to login or create account
    while(!success && !terminate)
    {
        ssize_t bytes_read;

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

        // build login or account create request
        make_login_create_req(&request_header, acc_create_login, form_type);

        // send packet that either represents account_create or login
        send_and_serialize_ACC_Create_Login(sockfd, &acc_create_login);

        // read the response into buffer
        bytes_read = read(sockfd, incoming_stream, PACKETLEN);
        if(bytes_read < 0)
        {
            perror("read");
            exit(EXIT_FAILURE);
        }
        incoming_stream[bytes_read] = '\0';

        // populate struct with response from server
        parse_response_header(incoming_stream, &incoming_message);

        // handle response of login/create account
        success = handle_login_create_res(incoming_message, incoming_stream, acc_create_login.username, sockfd);

        free_acc_create(&acc_create_login);
    }

    return *err;
}
