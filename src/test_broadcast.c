#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define HEADER_SIZE 6

struct CHT_Send {
    uint8_t *timestamp;
    uint8_t *content;
    uint8_t *username;
};

struct CHT_Send *read_chat_broadcast(const uint8_t *byte_stream);

int main() {

    uint8_t byte_stream[] = {
        0x14, 0x02, 0x00, 0x01, 0x00, 0x1E,
        0x18, 0x0F, '2', '0', '2', '4', '0', '3', '0', '1', '1', '2', '3', '0', '4', '5', 'Z',
        0x0C, 0x02, 'h', 'i',
        0x0C, 0x07, 'T', 'e', 's', 't', 'i', 'n', 'g'
    };

    size_t byte_stream_size = sizeof(byte_stream);

    printf("Byte Stream (%zu bytes):\n", byte_stream_size);
    for (size_t i = 0; i < byte_stream_size; i++) {
        printf("%02X ", byte_stream[i]);
    }
    printf("\n");

    struct CHT_Send *parsed_chat = read_chat_broadcast(byte_stream);
    if (!parsed_chat) {
        fprintf(stderr, "Failed to parse.\n");
        return EXIT_FAILURE;
    }

    printf("\nTimestamp: %.*s\n", byte_stream[7], parsed_chat->timestamp);
    printf("Content: %.*s\n", byte_stream[18], parsed_chat->content);
    printf("Username: %.*s\n", byte_stream[22], parsed_chat->username);

    free(parsed_chat->timestamp);
    free(parsed_chat->content);
    free(parsed_chat->username);
    free(parsed_chat);

    return EXIT_SUCCESS;
}

struct CHT_Send *read_chat_broadcast(const uint8_t *byte_stream)
{
    struct CHT_Send *incoming_chat;
    int              pos;
    uint8_t          length;

    incoming_chat = (struct CHT_Send *)malloc(sizeof(struct CHT_Send));
    if(!incoming_chat)
    {
        perror("malloc");
        return NULL;
    }

    pos                      = HEADER_SIZE + 1;
    length                   = byte_stream[pos++];
    incoming_chat->timestamp = (uint8_t *)malloc(length + (size_t)1);
    if(!incoming_chat->timestamp)
    {
        perror("malloc");
        free(incoming_chat);
        return NULL;
    }
    memcpy(incoming_chat->timestamp, &byte_stream[pos], length);
    incoming_chat->timestamp[length] = '\0';
    pos += length + 1;

    length                 = byte_stream[pos++];
    incoming_chat->content = (uint8_t *)malloc(length + (size_t)1);
    if(!incoming_chat->content)
    {
        perror("malloc");
        free(incoming_chat->timestamp);
        free(incoming_chat);
        return NULL;
    }
    memcpy(incoming_chat->content, &byte_stream[pos], length);
    incoming_chat->content[length] = '\0';
    pos += length + 1;

    length                  = byte_stream[pos++];
    incoming_chat->username = (uint8_t *)malloc(length);
    if(!incoming_chat->username)
    {
        perror("malloc");
        free(incoming_chat->timestamp);
        free(incoming_chat->content);
        free(incoming_chat);
        return NULL;
    }
    memcpy(incoming_chat->username, &byte_stream[pos], length);
    incoming_chat->username[length] = '\0';

    return incoming_chat;
}

