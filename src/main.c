#include "chat_screen.h"
#include "error_message.h"
#include "messages.h"
#include "packet.h"
#include "signals.h"
#include "signup_form.h"
#include "start_menu.h"
#include <ncurses.h>

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
