
#include "signup_form.h"
#include "gui.h"
#include "signals.h"
#include <ctype.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

// hellobuddyhowareyou

#define INPUT_BUFFER_SIZE 25
#define HEIGHT 10
#define WIDTH 40

void start_signup_form(struct ACC_Create_Login *acc_create, int setting, int *err)
{
    char    username[INPUT_BUFFER_SIZE];
    char    password[INPUT_BUFFER_SIZE];
    int     height;
    int     width;
    int     starty;
    int     startx;
    WINDOW *win;
    size_t  i;
    size_t  j;
    int     inputting_info;

    inputting_info = 1;

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(1);

    height = HEIGHT;
    width  = WIDTH;
    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;

    // Create a new window for the GUI
    win = newwin(height, width, starty, startx);
    draw_box(win);
    if(setting == 2)
    {
        mvwprintw(win, 1, 2, "Create Account");
    }
    else
    {
        mvwprintw(win, 1, 2, "Login");
    }

    // Input fields for username and password
    mvwprintw(win, 3, 2, "Username: ");    // NOLINT
    mvwprintw(win, 5, 2, "Password: ");    // NOLINT
    wrefresh(win);
    keypad(win, TRUE);

    wmove(win, 3, 12);    // NOLINT

    // used to determine how long username and password are
    i = 0;
    j = 0;

    while(inputting_info)
    {
        int ch;

        while((ch = wgetch(win)))
        {
            // make sure character is alpha numeric and within limit of username
            if(isalnum(ch) && i < sizeof(username) - 1)
            {
                username[i++] = (char)ch;
                waddch(win, (chtype)ch);    // NOLINT
                wrefresh(win);
            }
            // be able to backspace
            else if(ch == 127 || ch == KEY_BACKSPACE)    // NOLINT
            {
                if(i > 0)
                {
                    i--;
                    wmove(win, 3, 12 + (int)i);    // NOLINT
                    waddch(win, ' ');
                    wmove(win, 3, 12 + (int)i);    // NOLINT
                    wrefresh(win);
                }
            }
            else if(ch == KEY_DOWN)    // NOLINT
            {
                wmove(win, 5, 12 + (int)j);    // NOLINT
                break;
            }
            else if(ch == '\n')
            {
                wmove(win, 5, 12 + (int)j);    // NOLINT
                inputting_info = 0;
                break;
            }
        }

        while((ch = wgetch(win)))
        {    // read password character-by-character
            if(isalnum(ch) && j < sizeof(password) - 1)
            {
                password[j++] = (char)ch;
                waddch(win, '*');
                wrefresh(win);
            }
            else if(ch == 127 || ch == KEY_BACKSPACE)    // NOLINT
            {
                if(j > 0)
                {
                    j--;
                    wmove(win, 5, 12 + (int)j);    // NOLINT
                    waddch(win, ' ');
                    wmove(win, 5, 12 + (int)j);    // NOLINT
                    wrefresh(win);
                }
            }
            // temp solution press \ to switch input field
            else if(ch == KEY_UP)    // NOLINT
            {
                wmove(win, 3, 12 + (int)i);    // NOLINT
                break;
            }
            else if(ch == '\n')
            {
                wmove(win, 3, 12 + (int)i);    // NOLINT
                inputting_info = 0;
                break;
            }
        }

        if(inputting_info == 0)
        {
            if(i == 0 || j == 0)
            {
                inputting_info = 1;
            }
        }
    }

    username[i] = '\0';
    password[j] = '\0';

    delwin(win);
    endwin();

    convert_username_password(acc_create, username, password, err);
}
