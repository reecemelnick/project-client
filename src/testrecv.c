#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define PAYLOADINDEX 6
#define LENGTHINDEX 4
#define MAX_MSG_SIZE 256


void read_message(uint8_t **byte_stream, char *message_buffer);

int main()
{
    uint8_t packet[PAYLOADINDEX + MAX_MSG_SIZE] = {0};
    uint16_t packet_length_n;
    char message_buffer[MAX_MSG_SIZE];

    char *test_message = "Hello";
    int msg_len = strlen(test_message);

    packet_length_n = htons(msg_len);
    memcpy(&packet[LENGTHINDEX], &packet_length_n, sizeof(uint16_t));

    memcpy(&packet[PAYLOADINDEX], test_message, msg_len);

    uint8_t *packet_ptr = packet;

    read_message(&packet_ptr, message_buffer);

    return 0;
}

void read_message(uint8_t **byte_stream, char *message_buffer)
{
    int index;
    int packet_length;
    uint16_t packet_length_n;

    index = PAYLOADINDEX;

    // read in the packet length and store
    memcpy(&packet_length_n, (*byte_stream) + LENGTHINDEX, sizeof(uint16_t));
    packet_length_n = ntohs(packet_length_n);
    packet_length = (int) packet_length_n;

    // read through message and store?? stop at correct length
    for (int i = 0; i < packet_length; i++)
    {
        printf("%d", i);
        message_buffer[i] = (char) *((*byte_stream) + index + i);
    }
    

    // TESTPRINT (it works, can be deleted)
    message_buffer[packet_length] = '\0';
    printf("\n\nmsg: %s", message_buffer);
}