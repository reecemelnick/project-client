#include "start_menu.h"
#include "gui.h"
#include <ncurses.h>

#define HEIGHT 10
#define WIDTH 40

static void print_menu_options(WINDOW *menuWin, const int *highlight, const char **choices);
static void browse_options(const int *choice, int *highlight);

static void print_menu_options(WINDOW *menuWin, const int *highlight, const char **choices)
{
    for(int i = 0; i < 2; i++)
    {
        if(i == *highlight)
        {
            wattron(menuWin, A_REVERSE);
        }
        mvwprintw(menuWin, i + 5, 1, "%s", choices[i]);    // NOLINT
        wattroff(menuWin, A_REVERSE);
    }
}

static void browse_options(const int *choice, int *highlight)
{
    switch(*choice)
    {
        case KEY_UP:
            *highlight -= 1;
            break;
        case KEY_DOWN:
            *highlight += 1;
            break;
        default:
            break;
    }
    if(*highlight == 2)
    {
        *highlight = 0;
    }
    if(*highlight == -1)
    {
        *highlight = 1;
    }
}

int display_menu(void)
{
    int         height;
    int         width;
    int         starty;
    int         startx;
    WINDOW     *win;
    const char *choices[2] = {"Login", "Create Account"};
    int         highlight  = 0;
    int         choice     = -1;

    initscr();
    noecho();
    cbreak();
    curs_set(0);
    refresh();

    height = HEIGHT;
    width  = WIDTH;
    starty = (LINES - height) / 2;
    startx = (COLS - width) / 2;

    // Create a new window for the GUI
    win = newwin(height, width, starty, startx);
    draw_box(win);
    mvwprintw(win, 1, 2, "Select");
    keypad(win, TRUE);

    while(!terminate)
    {
        if(choice == KEY_RIGHT)
        {
            break;
        }
        print_menu_options(win, &highlight, choices);
        choice = wgetch(win);
        browse_options(&choice, &highlight);
        refresh();
    }
    delwin(win);
    endwin();
    return (highlight + 1);
}
