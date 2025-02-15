#include <ctype.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

int  start_chat_screen(uint16_t user_id);
void users_box(WINDOW *usersWin);
void chat_log_box(WINDOW *win);
void chat_input(WINDOW *win, uint16_t user_id);
