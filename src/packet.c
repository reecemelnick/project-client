#include "../include/packet.h"
#include "../include/messages.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define HEADER_SIZE 6
#define ID_INDEX 2
#define LENGTH_INDEX 4
#define TIMEOUT 100000

void construct_message(struct Message *header, uint8_t type, uint8_t version, uint16_t id, uint16_t length)
{
    header->packet_type = type;
    header->version     = version;
    header->sender_id   = id;
    header->payload_len = length;
}

void send_and_serialize_ACC_Create_Login(int serverfd, const struct ACC_Create_Login *packet)
{
    size_t packet_size;

    uint8_t *buffer;
    uint8_t *payload_buffer;

    uint16_t sender_id_n;
    uint16_t payload_len_n;

    // assign the packet size
    packet_size = (size_t)HEADER_SIZE + packet->message->payload_len;

    // malloc for each corresponding buffer
    buffer = (uint8_t *)malloc(packet_size);
    if(!buffer)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    payload_buffer = (uint8_t *)malloc(packet->message->payload_len);
    if(!payload_buffer)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // copy payload memory into the payload buffer
    memcpy(payload_buffer, packet->username, (size_t)packet->username[1] + 2);
    memcpy(payload_buffer + packet->username[1] + 2, packet->password, (size_t)packet->password[1] + 2);

    // convert the uint16_t attributes to network byte order
    sender_id_n   = htons(packet->message->sender_id);
    payload_len_n = htons(packet->message->payload_len);

    // assign the first 2 bytes of the packet (uint8_t)
    buffer[0] = packet->message->packet_type;
    buffer[1] = packet->message->version;

    // assign the next 4 bytes (uint16_t)
    memcpy(buffer + ID_INDEX, &sender_id_n, sizeof(uint16_t));
    memcpy(buffer + LENGTH_INDEX, &payload_len_n, sizeof(uint16_t));

    // assign the payload past the header
    memcpy(buffer + HEADER_SIZE, payload_buffer, packet->message->payload_len);

    printf("Sending packet of size %zu\n", packet_size);

    send_packet_t(buffer, packet_size);

    // send the buffer
    send_packet(serverfd, buffer, packet_size);

    free(payload_buffer);
    free(buffer);
}

void send_packet(const int serverfd, const uint8_t *buffer, const size_t size)
{
    // int bytes;

    if(write(serverfd, buffer, size) < 0)
    {
        perror("Write");
    }
}

void send_packet_t(const uint8_t *buffer, size_t packet_size)
{
    // Mock send function
    // printf("Sending packet of size %zu\n", packet_size);
    for(size_t i = 0; i < packet_size; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}

void construct_connection_message(struct ConnectionMessage *connection_message, const int message_type, const int version)
{
    connection_message->message_type = (uint8_t)message_type;
    connection_message->version      = (uint8_t)version;
}

void serialize_and_send_connection_message(const int serverfd, const struct ConnectionMessage *connection_message, int *err)
{
    uint8_t *buffer;
    // 1 byte for message_type, 1 byte for version
    size_t size = 2;
    buffer      = (uint8_t *)malloc(2 * sizeof(uint8_t));
    if(buffer == NULL)
    {
        *err = errno;
        perror("malloc");
        return;
    }

    // Assign first byte to message type
    buffer[0] = connection_message->message_type;
    // Assign second byte to version
    buffer[1] = connection_message->version;

    // Send the packet
    send_packet(serverfd, buffer, size);

    free(buffer);
}

uint8_t *read_entire_stream(const int serverfd, size_t *size, int *err)
{
    uint8_t       buffer[BUFFER_SIZE];
    uint8_t      *temp;
    struct pollfd pfd              = {serverfd, POLLIN, 0};
    size_t        total_bytes_read = 0;
    uint8_t      *entire_stream    = NULL;

    printf("Reading response\n");

    if(set_fd_non_blocking(serverfd) == -1)
    {
        *err = errno;
        perror("Failed to set serverfd to non-blocking");
        return NULL;
    }

    while(1)
    {
        ssize_t bytes_read = 0;

        int ret = poll(&pfd, 1, TIMEOUT);
        if(ret == 0)
        {
            continue;
        }

        if(ret < 0)
        {
            *err = errno;
            perror("poll");
            break;
        }

        if(pfd.revents & POLLIN)
        {
            bytes_read = read(serverfd, buffer, BUFFER_SIZE);
            if(bytes_read == -1)
            {
                if(errno == EAGAIN)
                {
                    printf("// No data available at the moment, continue polling");
                    continue;
                }
                *err = errno;
                perror("read");
                break;
            }
        }

        if(bytes_read > 0)
        {
            temp = (uint8_t *)realloc(entire_stream, total_bytes_read + (size_t)bytes_read);
            if(temp == NULL)
            {
                *err = errno;
                perror("realloc");
                if(entire_stream != NULL)
                {
                    free(entire_stream);
                }
                return NULL;
            }

            entire_stream = temp;
            memcpy(entire_stream + total_bytes_read, buffer, (size_t)bytes_read);
            total_bytes_read += (size_t)bytes_read;

            printf("Total bytes read: %zu\n", total_bytes_read);
        }

        break;
    }

    *size = total_bytes_read;
    printf("Done reading\n");

    return entire_stream;
}

uint16_t *get_user_id(const uint8_t *byte_stream)
{
    size_t    size     = 2;
    size_t    position = HEADER_SIZE + size;
    uint16_t *user_id;

    user_id = (uint16_t *)malloc(sizeof(uint16_t));
    if(!user_id)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    memcpy(user_id, byte_stream + position, size);

    *user_id = ntohs(*user_id);

    return user_id;
}

uint8_t *get_error_code(const uint8_t *byte_stream, size_t size)
{
    size_t   position = HEADER_SIZE + 2;
    uint8_t *err_code;

    err_code = (uint8_t *)malloc(size * sizeof(uint8_t));
    if(!err_code)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    memcpy(err_code, byte_stream + position, size);

    return err_code;
}

uint8_t *parse_and_extract_message(const uint8_t *byte_stream, size_t offset, size_t message_length, int *err)
{
    // allocate memory
    uint8_t *message = (uint8_t *)malloc(message_length * sizeof(uint8_t));
    if(message == NULL)
    {
        perror("malloc");
        *err = errno;
        return NULL;
    }

    // create a copy of byte_stream to work with, starting from offset
    memcpy(message, byte_stream + offset, message_length);
    return message;
}

uint8_t *parse_and_extract_payload_value(const uint8_t *byte_stream, size_t payload_value_size)
{
    // size_t length;
    // size_t   position = HEADER_SIZE;
    uint8_t *temp_byte_stream;
    uint8_t *payload_value;
    size_t   position = HEADER_SIZE;

    // create a non-constant byte stream to work with
    temp_byte_stream = (uint8_t *)malloc((position + payload_value_size) * sizeof(uint8_t));
    if(!temp_byte_stream)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    memcpy(temp_byte_stream, byte_stream, position + payload_value_size);

    // moves position to payload value
    position += 2;

    // store payload value
    payload_value = (uint8_t *)malloc(payload_value_size * sizeof(uint8_t));
    if(!payload_value)
    {
        perror("malloc");
        free(temp_byte_stream);
        exit(EXIT_FAILURE);
    }

    printf("pos: %d", (int)position);

    memcpy(payload_value, temp_byte_stream + position, payload_value_size);

    free(temp_byte_stream);
    return payload_value;
}

void parse_response_header(const uint8_t *byte_stream, struct Message *incoming_message)
{
    uint8_t  packet_type;
    uint8_t  version;
    uint16_t sender_id;
    size_t   position;
    uint16_t payload_len;

    position = 0;

    packet_type = byte_stream[position];

    incoming_message->packet_type = packet_type;
    ++position;

    version = byte_stream[position];
    if(version != 1)
    {
        printf("verion: create and send error packet");
        exit(EXIT_FAILURE);
    }
    incoming_message->version = version;
    ++position;

    sender_id = extract_next_twobytes(byte_stream, &position);

    if(sender_id != 0)
    {
        printf("id: create and send error packet");
        exit(EXIT_FAILURE);
    }
    incoming_message->sender_id = sender_id;

    payload_len                   = extract_next_twobytes(byte_stream, &position);
    incoming_message->payload_len = payload_len;
}

uint16_t extract_next_twobytes(const uint8_t *byte_stream, size_t *position)
{
    uint16_t twobytes;
    memcpy(&twobytes, byte_stream + *position, sizeof(uint16_t));
    twobytes = ntohs(twobytes);
    *position += 2;
    return twobytes;
}

int set_fd_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1)
    {
        perror("fcntl F_GETFL");
        return -1;
    }
    flags |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flags) == -1)
    {
        perror("fcntl F_SETFL");
        return -1;
    }
    return 0;
}
