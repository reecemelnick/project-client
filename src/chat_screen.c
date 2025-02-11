
#include "chat_screen.h"
#include "gui.h"
#include <ctype.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

int start_chat_screen(uint8_t *user_id)
{
    WINDOW *usersWin       = NULL;
    WINDOW *chat_log_win   = NULL;
    WINDOW *chat_input_win = NULL;

    initscr();
    cbreak();
    noecho();
    curs_set(1);

    users_box(usersWin);
    chat_log_box(chat_log_win);
    chat_input(chat_input_win, user_id);

    delwin(usersWin);
    delwin(chat_log_win);
    endwin();

    return 0;
}

void chat_input(WINDOW *win, uint8_t *user_id)
{
    int height;
    int width;
    int starty;
    int startx;

    height = 5;      // NOLINT
    width  = 111;    // NOLINT
    starty = 31;     // NOLINT
    startx = 1;

    win = newwin(height, width, starty, startx);
    draw_box(win);
    wmove(win, 1, 1);
    mvwprintw(win, 2, 2, "User ");    // NOLINT
    for(size_t i = 0; i < 2; i++)
    {
        wprintw(win, "%d", (int)user_id[i]);
    }
    mvwprintw(win, 2, 9, ":");    // NOLINT

    while(1)
    {
        int ch;

        while((ch = wgetch(win)))
        {
            waddch(win, (chtype)ch);
            wrefresh(win);
        }

        if(ch == '\n' || ch == 27)    // NOLINT
        {                             // Exit on ENTER or ESC
            break;
        }
    }

    wrefresh(win);
}

void users_box(WINDOW *usersWin)
{
    int height;
    int width;
    int starty;
    int startx;

    height = 30;    // NOLINT
    width  = 30;    // NOLINT
    starty = 1;
    startx = 1;

    usersWin = newwin(height, width, starty, startx);
    draw_box(usersWin);

    wrefresh(usersWin);
}

void chat_log_box(WINDOW *win)
{
    int height;
    int width;
    int starty;
    int startx;

    height = 30;    // NOLINT
    width  = 80;    // NOLINT
    starty = 1;
    startx = 32;    // NOLINT

    win = newwin(height, width, starty, startx);
    draw_box(win);

    wrefresh(win);
}
