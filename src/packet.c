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
