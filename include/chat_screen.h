#include <ctype.h>
#include <messages.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node
{
    // message data
    // cppcheck-suppress unusedStructMember
    char *data;
    // next message in list
    // cppcheck-suppress unusedStructMember
    struct Node *next;

} Node;

int  start_chat_screen(uint16_t user_id, uint8_t *username, int sockfd);
void generate_timestamp_byte_stream(uint8_t *byte_stream);
// void  get_generalized_time(uint8_t *buffer, size_t size);
