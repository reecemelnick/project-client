#include <ctype.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

int  start_chat_screen(uint16_t user_id, uint8_t *username, int sockfd);
void users_box(WINDOW *usersWin, uint8_t *username);
void chat_log_box(WINDOW **win);
void chat_input(WINDOW *win, uint16_t user_id, uint8_t *username, int sockfd);
int  confirm_CHT_success(const uint8_t *byte_stream);
