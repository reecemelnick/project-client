#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

#define PACKETLEN 777

void send_user_message(int fd, const char *message_buffer, uint8_t type, uint8_t ver, uint16_t id, uint16_t length)
{
    uint8_t  buffer[PACKETLEN];
    uint16_t sender_id_n;
    uint16_t payload_n;
    int      pos;

    pos = 0;

    // assign type
    buffer[pos] = type;
    pos++;

    // assign version
    buffer[pos] = ver;
    pos++;

    // assign id
    sender_id_n = htons(id);
    memcpy(buffer + pos, &sender_id_n, sizeof(uint16_t));
    pos += (int)sizeof(uint16_t);

    // assign len
    payload_n = htons(length);
    memcpy(buffer + pos, &payload_n, sizeof(uint16_t));
    pos += (int)sizeof(uint16_t);

    strlcat((char *)(buffer + pos), message_buffer, PACKETLEN - (size_t)pos);

    // disabled for testing
    // send_packet(fd, buffer, sizeof(buffer));

    
    printf("Packet sent with message: %s\n", buffer + pos);
}

int main() {
    // Test data
    char message[] = "Hello, this is a test message!";
    uint8_t type = 20;   
    uint8_t ver = 1;     
    uint16_t id = 1;  
    uint16_t length = strlen(message); 

    send_user_message(0, message, type, ver, id, length);

    return 0;
}

