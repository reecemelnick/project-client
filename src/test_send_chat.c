#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#define PACKETLEN 4096
#define HEADER_SIZE 6

struct Message {
    // cppcheck-suppress unusedStructMember
    uint8_t packet_type;
    // cppcheck-suppress unusedStructMember
    uint8_t version;
    // cppcheck-suppress unusedStructMember
    uint16_t sender_id;
    // cppcheck-suppress unusedStructMember
    uint16_t payload_len;
};

struct CHT_Send {
    // cppcheck-suppress unusedStructMember
    struct Message *message;
    // cppcheck-suppress unusedStructMember
    uint8_t *timestamp;
    // cppcheck-suppress unusedStructMember
    uint8_t *content;
    // cppcheck-suppress unusedStructMember
    uint8_t *username;
};


void send_user_message(int fd, struct CHT_Send *cht_packet);

uint8_t *construct_cht_payload(struct CHT_Send *cht_packet);

void print_packet(int fd, uint8_t *buffer, size_t size);

void send_packet_t(const uint8_t *buffer, size_t packet_size);

int main() {
    // Sample data
    uint8_t timestamp[] = { 0x18, 0x0F, '2', '0', '2', '4', '0', '3', '0', '1', '1', '2', '3', '0', '4', '5', 'Z' };
    uint8_t content[]   = { 0x0C, 0x02, 'h', 'i' };
    uint8_t username[]  = { 0x0C, 0x07, 'T', 'e', 's', 't', 'i', 'n', 'g' };

    struct Message msg = { 0x14, 0x02, 0x01, 0x1E };

    struct CHT_Send chat_packet;
    chat_packet.message = &msg;
    chat_packet.timestamp = timestamp;
    chat_packet.content = content;
    chat_packet.username = username;

    // Extract sizes from second byte
    size_t timestamp_size = timestamp[1];
    size_t content_size = content[1];
    size_t username_size = username[1];

    // Print data before calling send_user_message
    printf("Timestamp (%zu bytes): ", timestamp_size);
    for (size_t i = 2; i < timestamp_size + 2; i++) {
        printf("%c", chat_packet.timestamp[i]);  // ASCII
    }
    printf("\n");

    printf("Content (%zu bytes): ", content_size);
    for (size_t i = 2; i < content_size + 2; i++) {
        printf("%c", content[i]);  // ASCII
    }
    printf("\n");

    printf("Username (%zu bytes): ", username_size);
    for (size_t i = 2; i < username_size + 2; i++) {
        printf("%c", username[i]);  // ASCII
    }
    printf("\n");

    // Call function (fd = 0 for testing)
    send_user_message(0, &chat_packet);

    return 0;
}


uint8_t *construct_cht_payload(struct CHT_Send *cht_packet)
{
    uint8_t *payload;

    size_t timestamp_size = (size_t) cht_packet->timestamp[1] + 2;
    size_t content_size = (size_t) cht_packet->content[1] + 2;
    size_t username_size = (size_t) cht_packet->username[1] + 2;

    payload = malloc(timestamp_size + content_size + username_size);
    if(payload == NULL)
    {
        perror("malloc");
        return NULL;
    }

    memcpy(payload, cht_packet->timestamp, timestamp_size);
    memcpy(payload + (int)timestamp_size, cht_packet->content, content_size);
    memcpy(payload + (int) timestamp_size + (int) content_size, cht_packet->username, username_size);

    return payload;
}

void send_user_message(int fd, struct CHT_Send *cht_packet)
{
    uint8_t  buffer[PACKETLEN];
    uint8_t *payload;
    uint16_t sender_id_n;
    uint16_t payload_n;
    size_t buffer_size;
    int      pos;

    pos = 0;

    // assign type
    buffer[pos] = cht_packet->message->packet_type;
    pos++;

    // assign version
    buffer[pos] = cht_packet->message->version;
    pos++;

    // assign id
    sender_id_n = htons(cht_packet->message->sender_id);
    memcpy(buffer + pos, &sender_id_n, sizeof(uint16_t));
    pos += (int)sizeof(uint16_t);

    // assign len
    payload_n = htons(cht_packet->message->payload_len);
    memcpy(buffer + pos, &payload_n, sizeof(uint16_t));
    pos += (int)sizeof(uint16_t);

    // assign payload
    payload = construct_cht_payload(cht_packet);
    memcpy(buffer + pos, payload, cht_packet->message->payload_len);
    free(payload);

    buffer_size = HEADER_SIZE + buffer[HEADER_SIZE - 1];

    // sends packet at the end
    print_packet(fd, buffer, buffer_size);
}

void send_packet_t(const uint8_t *buffer, size_t packet_size)
{
    for(size_t i = 0; i < packet_size; i++)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}

void print_packet(int fd, uint8_t *buffer, size_t size) 
{
    printf("CONTENT LENGTH: %d\n\n", buffer[5]);
    printf("Packet Sent (size %zu bytes):\n", size);
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", buffer[i]);
    }

    printf("\n");
}