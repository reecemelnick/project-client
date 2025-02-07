#include "../include/packet.h"
#include "../include/messages.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define HEADER_SIZE 6
#define ID_INDEX 2
#define LENGTH_INDEX 4

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
    buffer         = (uint8_t *)malloc(packet_size);
    payload_buffer = (uint8_t *)malloc(packet->message->payload_len);

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

uint8_t *read_entire_stream(const int serverfd, int *err)
{
    uint8_t  buffer[BUFFER_SIZE];
    ssize_t  bytes_read       = 0;
    size_t   total_bytes_read = 0;
    uint8_t *entire_stream    = NULL;

    // read 1024 bytes at a time
    while((bytes_read = read(serverfd, buffer, BUFFER_SIZE)) > 0)
    {    // TODO: implement sigint handling

        // realloc another bytes_read number of bytes to temp
        uint8_t *temp = (uint8_t *)realloc(entire_stream, total_bytes_read + (size_t)bytes_read);
        if(temp == NULL)    // if realloc failed
        {
            *err = errno;
            perror("realloc");
            if(entire_stream != NULL)
            {
                free(entire_stream);
            }
            return NULL;
        }
        // if realloc was a success, assign to entire_stream
        entire_stream = temp;
        // appends data in buffer to entire_stream
        memcpy(entire_stream + total_bytes_read, buffer, (size_t)bytes_read);
        total_bytes_read += (size_t)bytes_read;
    }

    if(bytes_read == -1)
    {
        *err = errno;
        perror("read");
        if(entire_stream != NULL)
        {
            free(entire_stream);
        }
        return NULL;
    }

    return entire_stream;
}

// void parse_response_acc_create(const uint8_t *byte_stream, int *err)
// {
//     uint8_t *payload;
//     int position = HEADER_SIZE;

//     for(size_t yy = 0; yy < size; yy++)
//     {
//         printf("%02X ", byte_stream[yy]);    // %02X for hex with leading zero
//     }
//     printf("\n");

//     printf("%02X\n", byte_stream[HEADER_SIZE]);
// }

uint16_t *parse_response_header(const uint8_t *byte_stream)
{
    uint8_t   packet_type;
    uint8_t   version;
    uint16_t  sender_id;
    uint16_t *payload_len = (uint16_t *)malloc(sizeof(uint16_t));
    size_t    position    = 0;

    packet_type = byte_stream[position];
    if(packet_type != SYS_Success && packet_type != SYS_Error)    // if packet_type is invalid then
    {
        printf("create and send error packet");
        free(payload_len);
        return NULL;
    }
    ++position;

    version = byte_stream[position];
    if(version != 1)    // if incorrect version
    {
        printf("create and send error packet");
        free(payload_len);
        return NULL;
    }

    sender_id = extract_next_twobytes(byte_stream, &position);
    if(sender_id != 0)    // if sender_id not set to 0
    {
        printf("create and send error packet");
        free(payload_len);
        return NULL;
    }

    *payload_len = extract_next_twobytes(byte_stream, &position);

    return payload_len;
}

uint16_t extract_next_twobytes(const uint8_t *byte_stream, size_t *position)
{
    uint16_t twobytes;
    memcpy(&twobytes, byte_stream + *position, sizeof(uint16_t));
    twobytes = ntohs(twobytes);
    *position += 2;
    return twobytes;
}
