#include <ctype.h>
#include <messages.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node
{
    // message data
    char *data;
    // next message in list
    struct Node *next;

} Node;

int   start_chat_screen(uint16_t user_id, uint8_t *username, int sockfd);
void  users_box(WINDOW *usersWin, uint8_t *username);
void  chat_log_box(WINDOW **win, WINDOW **inner);
void  chat_input(WINDOW *win, uint16_t user_id, uint8_t *username, int sockfd);
int   confirm_CHT_success(const uint8_t *byte_stream);
void  generate_timestamp_byte_stream(uint8_t *byte_stream);
void *chat_log_thread(void *arg);
void  get_generalized_time(uint8_t *buffer, size_t size);
void  make_chat_input_box(WINDOW **win, char *username, uint16_t user_id, int *cursor_pos);
Node *add_message_to_LL(const uint8_t *message, const uint8_t *username);
void  free_nodes(Node *head_node);
void  clear_text_window(WINDOW *win);
void  show_messages(WINDOW *win, Node *head);
Node *shift_nodes(Node *head);
void  build_node_data(Node *new_node, const uint8_t *message, const uint8_t *username, size_t message_len, size_t username_len);
Node *allocate_node_data(size_t total_len);
Node *initialize_head_node(void);
void  free_CHT_SEND(struct CHT_Send *chat_message);
void  cleanup_input_chat(struct CHT_Send new_chat);
