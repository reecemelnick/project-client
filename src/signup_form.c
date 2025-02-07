
#include "signup_form.h"
#include "gui.h"
#include <ctype.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

// hellobuddyhowareyou

#define INPUT_BUFFER_SIZE 50
#define HEIGHT 10
#define WIDTH 40

void start_signup_form(struct ACC_Create_Login *acc_create, int setting, int *err)
{
    char    username[INPUT_BUFFER_SIZE];
    char    password[INPUT_BUFFER_SIZE];
    char   *user;
    char   *pass;
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
            // temp solution press \ to switch input field
            else if(ch == KEY_DOWN || ch == '\n')    // NOLINT
            {
                wmove(win, 5, 12 + (int)j);    // NOLINT
                break;
            }
        }

        if(j == 0)
        {
            wmove(win, 5, 12);    // NOLINT
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
                break;
            }
        }

        if(i == 0 || j == 0)
        {
            inputting_info = 1;
            wmove(win, 3, 12 + (int)i);    // NOLINT
        }
        else
        {
            break;
        }
    }

    username[i] = '\0';
    password[j] = '\0';

    user = (char *)malloc(strlen(username));
    if(!user)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    pass = (char *)malloc(strlen(password));
    if(!pass)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    memcpy(user, username, strlen(username));
    memcpy(pass, password, strlen(password));

    // clear the window and display the entered details
    werase(win);
    draw_box(win);
    mvwprintw(win, 2, 2, "Account Details:");
    mvwprintw(win, 4, 2, "Username: %s, len: %d", username, (int)strlen(username));    // NOLINT
    mvwprintw(win, 6, 2, "Password: %s, len: %d", password, (int)strlen(password));    // NOLINT
    wrefresh(win);

    // wait for the user to press a key before exiting
    mvwprintw(win, 8, 2, "Press any key to exit...");    // NOLINT
    wrefresh(win);
    wgetch(win);

    delwin(win);
    endwin();

    convert_username_password(acc_create, user, pass, err);
}
