#include <ctype.h>
#include <messages.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

int   start_chat_screen(uint16_t user_id, uint8_t *username, int sockfd);
void  users_box(WINDOW *usersWin, uint8_t *username);
void  chat_log_box(WINDOW **win, WINDOW **inner);
void  chat_input(WINDOW *win, uint16_t user_id, uint8_t *username, int sockfd);
int   confirm_CHT_success(const uint8_t *byte_stream);
void  build_chat_struct(struct CHT_Send *new_chat, struct Message *chat_header, const uint8_t *message, const uint8_t *username, uint16_t id);
void  generate_timestamp_byte_stream(uint8_t *byte_stream);
void *chat_log_thread(void *arg);
void  get_generalized_time(uint8_t *buffer, size_t size);
