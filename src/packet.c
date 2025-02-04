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

void serialize_and_send_message(const int serverfd, const struct Message *header, uint8_t username, uint8_t password)
{
    size_t packet_size;

    uint8_t *buffer;
    uint8_t *payload_buffer;

    uint16_t sender_id_n;
    uint16_t payload_len_n;

    // assign the packet size
    packet_size = (size_t)HEADER_SIZE + header->payload_len;

    // malloc for each corresponding buffer
    buffer         = (uint8_t *)malloc(packet_size);
    payload_buffer = (uint8_t *)malloc(header->payload_len);

    // copy payload memory into the payload buffer
    memcpy(payload_buffer, &username, sizeof(username));
    memcpy(payload_buffer + sizeof(username), &password, sizeof(password));

    // convert the uint16_t attributes to network byte order
    sender_id_n   = htons(header->sender_id);
    payload_len_n = htons(header->payload_len);

    // assign the first 2 bytes of the packet (uint8_t)
    buffer[0] = header->packet_type;
    buffer[1] = header->version;

    // assign the next 4 bytes (uint16_t)
    memcpy(buffer + ID_INDEX, &sender_id_n, sizeof(uint16_t));
    memcpy(buffer + LENGTH_INDEX, &payload_len_n, sizeof(uint16_t));

    // assign the payload past the header
    memcpy(buffer + HEADER_SIZE, payload_buffer, sizeof(header->payload_len));

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
